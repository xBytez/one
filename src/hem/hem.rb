#!/usr/bin/env ruby

# -------------------------------------------------------------------------- #
# Copyright 2002-2019, OpenNebula Project, OpenNebula Systems                #
#                                                                            #
# Licensed under the Apache License, Version 2.0 (the "License"); you may    #
# not use this file except in compliance with the License. You may obtain    #
# a copy of the License at                                                   #
#                                                                            #
# http://www.apache.org/licenses/LICENSE-2.0                                 #
#                                                                            #
# Unless required by applicable law or agreed to in writing, software        #
# distributed under the License is distributed on an "AS IS" BASIS,          #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
# See the License for the specific language governing permissions and        #
# limitations under the License.                                             #
#--------------------------------------------------------------------------- #

require 'rubygems'
require 'ffi-rzmq'
require 'nokogiri'
require 'yaml'
require 'logger'
require 'base64'

ONE_LOCATION = ENV['ONE_LOCATION']

if !ONE_LOCATION
    LOG_LOCATION = '/var/log/one'
    VAR_LOCATION = '/var/lib/one'
    ETC_LOCATION = '/etc/one'
    LIB_LOCATION = '/usr/lib/one'
    RUBY_LIB_LOCATION = '/usr/lib/one/ruby'
else
    VAR_LOCATION = ONE_LOCATION + '/var'
    LOG_LOCATION = ONE_LOCATION + '/var'
    ETC_LOCATION = ONE_LOCATION + '/etc'
    LIB_LOCATION = ONE_LOCATION + '/lib'
    RUBY_LIB_LOCATION = ONE_LOCATION + '/lib/ruby'
end

$LOAD_PATH << RUBY_LIB_LOCATION

require 'opennebula'
require 'CommandManager'
require 'ActionManager'

# Hook Execution Manager class
class HookExecutionManager

    attr_reader :am

    # API calls which trigger hook info reloading
    UPDATE_CALLS = [
        'one.hook.update',
        'one.hook.allocate',
        'one.hook.delete'
    ]

    # List of filters for API calls which trigger hooks reload (TODO, load it from config file?)
    STATIC_FILTERS = [
        'API one.hook.update 1',
        'API one.hook.allocate 1',
        'API one.hook.delete 1'
    ]

    DEBUG_LEVEL = [
        Logger::ERROR, # 0
        Logger::WARN,  # 1
        Logger::INFO,  # 2
        Logger::DEBUG  # 3
    ]

    CONFIGURATION_FILE = ETC_LOCATION + '/hem.conf'
    HEM_LOG            = LOG_LOCATION + '/hem.log'

    HOOK_TYPES         = [:api, :state]

    MSG_FORMAT         = %(%s [%s]: %s\n) # Mon Feb 27 06:02:30 2012 [Clo] [E]: Error message example
    DATE_FORMAT        = '%a %b %d %H:%M:%S %Y' # Mon Feb 27 06:02:30 2012

    def initialize
        @conf       = nil
        @logger     = nil

        # 0mq related variables
        @context    = ZMQ::Context.new(1) # 0mq context (shared between all the sockets)
        @subscriber = nil # 0mq SUBSCRIBER socket
        @requester  = nil # 0mq REQUESTER socket

        @hooks      = {}  # Dictionary containing all the existing Hooks
        @filters    = {}  # Dictionary conatinitn a filter for each existing Hook

        # 0mq related variables
        @context    = ZMQ::Context.new(1) # 0mq context (shared between all the sockets)
        @subscriber = @context.socket(ZMQ::SUB) # 0mq SUBSCRIBER socket
        @requester  = @context.socket(ZMQ::REQ) # 0mq REQUESTER socket

        load_config
        init_log

        # Action manager initialization
        # TODO, make AM configurable via config file
        @am = ActionManager.new(2, true)
        @am.register_action(:EXECUTE, method('execute_action'))
    end

    def load_config
        # Load config from configuration file
        begin
            @conf = YAML.load_file(CONFIGURATION_FILE)
        rescue Exception => e
            STDERR.puts "Error parsing config file #{CONFIGURATION_FILE}: #{e.message}"
            exit 1
        end

        # Set default values if empty
        @conf[:hook_base_path] ||= '/var/lib/one/remotes/hooks'
        @conf[:subscriber_endpoint] ||= 'tcp://localhost:5556'
        @conf[:replier_endpoint] ||= 'tcp://localhost:5557'
        @conf[:debug_level] ||= 2
    end

    def init_log
        @logger = Logger.new(HEM_LOG)
        @logger.level = DEBUG_LEVEL[@conf[:debug_level].to_i]
        @logger.formatter = proc do |severity, datetime, _progname, msg|
            format(MSG_FORMAT,
                datetime.strftime(DATE_FORMAT),
                severity[0..0],
                msg)
        end
    end

    ##############################################################################
    # Helpers
    ##############################################################################

    # Generates a valid oca client
    def gen_client
        # TODO, create the client just once?
        client = nil

        if !@conf[:auth].nil?
            client = OpenNebula::Client.new(@conf[:auth], @conf[:one_xmlrpc])
        else
            client = OpenNebula::Client.new(nil, @conf[:one_xmlrpc])
        end

        client
    end

    # Generates a key for a given hook
    #
    # @param hook [Hook] Hook object
    #
    # @return key corresponded to the given hookd
    def get_key(hook)
        type = hook['TYPE'].to_sym

        return hook['TEMPLATE/CALL'] if type == HOOK_TYPES[0] #API

        return "#{hook['//RESOURCE']}/#{hook['//STATE']}/#{hook['//LCM_STATE']}" if type == HOOK_TYPES[1]  #STATE
    end

    # Subscribe the subscriber socket to the given filter
    def subscribe(filter)
        # TODO, check params
        @subscriber.setsockopt(ZMQ::SUBSCRIBE, filter)
    end

    # Unsubscribe the subscriber socket from the given filter
    def unsubscribe(filter)
        # TODO, check params
        @subscriber.setsockopt(ZMQ::UNSUBSCRIBE, filter)
    end

    # Load Hooks from oned (one.hookpool.info) into a dictionary with the
    # following format:
    #
    # hooks[hook_type][hook_key] = Hook object
    #
    # Also generates and store the corresponding filters
    #
    # @return dicctionary containing hooks dictionary and filters
    def load_hooks_info
        client = gen_client

        @logger.info('Loading Hooks...')

        # TODO, manage errors
        hook_pool = OpenNebula::HookPool.new(client)
        hook_pool.info

        HOOK_TYPES.each do |type|
            @hooks[type] = {}
        end

        hook_pool.each do |hook|
            type = hook['TYPE'].to_sym

            if !HOOK_TYPES.include? type
                @logger.error("Error loading hooks. Invalid hook type: #{type}")
                next
            end

            key = get_key(hook)

            @hooks[type][key] = hook

            @filters[hook['ID'].to_i] = gen_filter(type, key, hook)
        end

        @logger.info('Hooks successfully loaded')
    end

    # Generate a subscriber filter for an API type hook
    def gen_filter_api(key, _hook)
        # TODO, subscribed for success, fail, always
        "API #{key} 1"
    end

    # Generate a sbuscriber filter for a STATE type hook
    def gen_filter_state(key, _hook)
        # TODO
        "STATE #{key}"
    end

    # Generate a sbuscriber filter for a Hook given the type and the key
    def gen_filter(type, key, hook)
        return gen_filter_api(key, hook)   if type == HOOK_TYPES[0]
        return gen_filter_state(key, hook) if type == HOOK_TYPES[1]
    end

    ############################################################################
    # Hook manager methods
    ############################################################################

    # Parse hook arguments
    def parse_args(args, key, content)
        params = ''
        alloc = key[1].include? 'allocate'

        if args.nil?
            return params
        end

        args = args.split ' '
        args.each do |arg|
            if arg == '$API'
                if alloc
                    params << ' ' << content.split(' ')[0..-2].join(' ')
                else
                    params << ' ' << content
                end

                next
            elsif arg == '$ID' && alloc
                params << ' ' << content.split(' ')[-1]

                next
            elsif arg == '$TEMPLATE'
                # TODO, get template (API call, auth?)
                next
            end
        end

        params
    end

    # Subscribe the socket to all the fileters included in filters and STATIC_FILTERS
    def setup_filters
        # Subscribe to hooks modifier calls
        STATIC_FILTERS.each do |filter|
            subscribe(filter)
        end

        # Subscribe to each existing hook
        @filters.each do |filter|
            subscribe(filter[1])
        end
    end

    # Reload a hook or deleted if needed
    def reload_hook(call, call_info)
        client   = gen_client
        info_xml = Nokogiri::XML(call_info)
        id = -1

        # TODO, what happens if not int?
        if call == 'one.hook.allocate'
            id = info_xml.xpath('//RESPONSE/OUT2')[0].text.to_i
        else
            id = info_xml.xpath('//PARAMETERS/PARAMETER2')[0].text.to_i
        end

        # Remove filter if the hook have been deleted or updated
        if call != 'one.hook.allocate'
            unsubscribe(@filters[id])
            @filters.delete(id)
        end

        return if call == 'one.hook.delete'

        # get new hook info
        hook = OpenNebula::Hook.new_with_id(id, client)
        rc = hook.info

        if !rc.nil?
            @logger.error("Error getting hook #{id}.")
            return
        end

        # Generates key and filter for the new hook info
        key    = get_key(hook)
        filter = gen_filter(hook['TYPE'].to_sym, key, hook)

        # Add new filter
        subscribe(filter)

        @hooks[hook['TYPE'].to_sym][key] = hook
        @filters[id] = filter
    end

    def execute_hook(hook, params)
        remote = hook['TEMPLATE/REMOTE'].casecmp('YES').zero?
        command = "#{@conf[:hook_base_path]}/#{hook['TEMPLATE/COMMAND']} #{params}"

        return LocalCommand.run(command) unless remote

        SSHCommand.run(command, hook['TEMPLATE/REMOTE_HOST'])
    end
    ############################################################################
    # Hook Execution Manager main loop
    ############################################################################

    def hem_loop
        # Connect subscriber socket for receiving the events
        @subscriber.connect(@conf[:subscriber_endpoint])

        # Connect requester socket for returning back the execution result
        @requester.connect(@conf[:replier_endpoint])

        # Initialize @hooks and @filters
        load_hooks_info

        # Set up subscriber filters
        setup_filters

        loop do
            key = ''
            content = ''

            @subscriber.recv_string(key)
            @subscriber.recv_string(content)

            key = key.split(' ')
            content = Base64.decode64(content)

            # It would be the same for state hooks?
            hook = @hooks[key[0].downcase.to_sym][key[1]]

            @am.trigger_action(:EXECUTE, 0, hook, key, content) unless hook.nil?

            next unless UPDATE_CALLS.include? key[1]

            @logger.info('Reloading Hooks...')
            reload_hook(key[1], content)
            @logger.info('Hooks Successfully reloaded')
        end
    end

    def execute_action(hook, key, content)
        ack = ''

        # trigger hook (get params, execute, return exec result)
        params = parse_args(hook[:args], key, content)
        # rc = system("#{@conf[:hook_base_path]}/#{hook['TEMPLATE/COMMAND']} #{params}")
        # TODO, manage stdin an stdout
        exec_result = execute_hook(hook, params)

        if exec_result.code.zero?
            @logger.info("Hook successfully executed for #{key[1]}")
        else
            @logger.error("Failure executing hook for #{key[1]}")
        end

        @requester.send_string("#{exec_result.code} '#{hook['NAME']}'" \
                                " #{hook['ID']} #{params}")
        @requester.recv_string(ack)

        @logger.error('Error receiving confirmation from hook manager.') if ack != 'ACK'
    end

    def start
        hem_thread = Thread.new { hem_loop }
        @am.start_listener
        hem_thread.kill
    end

end

hem = HookExecutionManager.new
hem.start

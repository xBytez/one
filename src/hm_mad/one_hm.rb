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

ONE_LOCATION = ENV['ONE_LOCATION']

if !ONE_LOCATION
    RUBY_LIB_LOCATION = '/usr/lib/one/ruby'
    ETC_LOCATION = '/etc/one/'
else
    RUBY_LIB_LOCATION = ONE_LOCATION + '/lib/ruby'
    ETC_LOCATION = ONE_LOCATION + '/etc/'
end

$LOAD_PATH << RUBY_LIB_LOCATION

require 'OpenNebulaDriver'
require 'getoptlong'
require 'rubygems'
require 'ffi-rzmq'

# HookManagerDriver class
class HookManagerDriver < OpenNebulaDriver

    def initialize(options)
        @options = {
            :concurrency => 15,
            :threaded => true,
            :retries => 0
        }.merge!(options)

        super('', @options)

        # Initialize publisher socket
        context = ZMQ::Context.new(1)
        @publisher = context.socket(ZMQ::PUB)
        @publisher.setsockopt(ZMQ::SNDHWM, @options[:hwm]) unless @options[:hwm].nil?

        @replier = context.socket(ZMQ::REP)

        # TODO, make the port configurable and add HWM option
        @publisher.bind("tcp://127.0.0.1:#{@options[:publisher_port]}")
        @replier.bind("tcp://127.0.0.1:#{@options[:logger_port]}")

        register_action(:EXECUTE, method('action_execute'))

        Thread.new do
            execution_result_thread
        end
    end

    def action_execute(type, *arguments)
        key = "#{type} #{arguments.shift(2).join(' ')}"
        vals = arguments.join(' ')

        # Using envelopes for splitting key/val (http://zguide.zeromq.org/page:all#Pub-Sub-Message-Envelopes)
        @publisher.send_string key, ZMQ::SNDMORE
        @publisher.send_string vals
    end

    def execution_result_thread
        Kernel.loop do
            execution = ''
            result    = ''

            rc = @replier.recv_string(execution)

            if rc < 0
                exit -1
            else
                @replier.send_string("ACK")
            end

            execution = execution.split(' ')
            if execution.shift.to_i == 1
                result = 'SUCCESS'
            else
                result = 'FAILURE'
            end

            puts "EXECUTE #{result} #{execution.flatten}"
        end
    end

end

# Hook Manager main program

opts = GetoptLong.new(
    ['--threads',        '-t', GetoptLong::OPTIONAL_ARGUMENT],
    ['--publisher-port', '-p', GetoptLong::OPTIONAL_ARGUMENT],
    ['--logger-port',    '-l', GetoptLong::OPTIONAL_ARGUMENT],
    ['--hwm',            '-h', GetoptLong::OPTIONAL_ARGUMENT]
)

threads        = 15
publisher_port = 5556
logger_port    = 5557
hwm            = nil # http://zguide.zeromq.org/page:all#High-Water-Marks

begin
    opts.each do |opt, arg|
        case opt
        when '--threads'
            threads = arg.to_i
        when '--publisher-port'
            publisher_port = arg.to_i
        when '--logger-port'
            logger_port = arg.to_i
        when '--hwm'
            hwm = arg.to_i
        end
    end
rescue Exception => e
    exit(-1)
end

hm = HookManagerDriver.new(:concurrency => threads,
                           :publisher_port => publisher_port,
                           :logger_port => logger_port,
                           :hwm => hwm)

hm.start_driver

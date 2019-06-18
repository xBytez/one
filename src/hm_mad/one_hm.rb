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

ONE_LOCATION=ENV["ONE_LOCATION"]

if !ONE_LOCATION
    RUBY_LIB_LOCATION="/usr/lib/one/ruby"
    ETC_LOCATION="/etc/one/"
else
    RUBY_LIB_LOCATION=ONE_LOCATION+"/lib/ruby"
    ETC_LOCATION=ONE_LOCATION+"/etc/"
end

$: << RUBY_LIB_LOCATION

require 'OpenNebulaDriver'

class HookManagerDriver < OpenNebulaDriver
    def initialize(options)
        @options={
            :concurrency => 15,
            :threaded => true,
            :retries => 0
        }.merge!(options)

        super('', @options)

        register_action(:EXECUTE, method("action_execute"))
    end

    def action_execute(type, *arguments)
        File.open("/tmp/driver_output", 'a') do |file|
            file.write("Type: #{type}\nArgs: #{arguments}\n=============================\n")
        end
    end
end

hm=HookManagerDriver.new(:concurrency => 15)
hm.start_driver

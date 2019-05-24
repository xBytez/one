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

require 'one_helper'
require 'rubygems'

# implements onehook command
class OneHookHelper < OpenNebulaHelper::OneHelper

    def self.rname
        'HOOK'
    end

    def self.conf_file
        'onehook.yaml'
    end

    private

    def factory(id = nil)
        if id
            OpenNebula::Hook.new_with_id(id, @client)
        else
            xml = OpenNebula::Hook.build_xml
            OpenNebula::Hook.new(xml, @client)
        end
    end

    def factory_pool(_user_flag = -2)
        OpenNebula::HookPool.new(@client)
    end

    def format_pool(_options)
        config_file = self.class.table_conf

        table = CLIHelper::ShowTable.new(config_file, self) do
            column :ID, 'ONE identifier for the Hook', :size => 5 do |d|
                d['ID']
            end

            column :NAME, 'Name of the Hook', :left, :size => 25 do |d|
                d['NAME']
            end

            column :TYPE, 'Type of the Hook', :left, :size => 45 do |d|
                d['TYPE']
            end

            default :ID, :NAME, :TYPE
        end

        table
    end

    def format_resource(hook, _options = {})
        str = '%-18s: %-20s'
        str_h1 = '%-80s'

        CLIHelper.print_header(str_h1 % "HOOK #{hook['ID']} INFORMATION")
        puts format str, 'ID',   hook.id.to_s
        puts format str, 'NAME', hook.name
        puts format str, 'TYPE', hook['TYPE']
        puts format str, 'LOCK', OpenNebulaHelper.level_lock_to_str(hook['LOCK/LOCKED']) # rubocop:disable Metrics/LineLength
        puts

        CLIHelper.print_header(str_h1 % 'HOOK TEMPLATE', false)
        puts hook.template_str

        puts
    end

end

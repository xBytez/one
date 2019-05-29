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
require 'CommandManager'
require 'base64'


# The SSH Information Manager Driver
class DummyInformationManager < OpenNebulaDriver

    # Init the driver
    def initialize(num)
        super('',
            :concurrency => num,
            :threaded => true
        )

        # register actions
        register_action(:MONITOR, method("action_monitor"))
        register_action(:STOPMONITOR, method("stop_monitor"))
    end

    # Execute the sensor array in the remote host
    def action_monitor(number, host, not_used1, not_used2)
        results =  "HYPERVISOR=dummy\n"
        results << "HOSTNAME=#{host}\n"

        results << "CPUSPEED=2.2GHz\n"

        used_memory = rand(16777216)
        results << "TOTALMEMORY=16777216\n"
        results << "USEDMEMORY=#{used_memory}\n"
        results << "FREEMEMORY=#{16777216-used_memory}\n"

        used_cpu = rand(800)
        results << "TOTALCPU=800\n"
        results << "USEDCPU=#{used_cpu}\n"
        results << "FREECPU=#{800-used_cpu}\n"

        results << "DS_LOCATION_USED_MB=9720\n"
        results << "DS_LOCATION_TOTAL_MB=20480\n"
        results << "DS_LOCATION_FREE_MB=20480\n"

        results << "PCI = [
                ADDRESS = \"0000:02:00:0\",\n
                BUS = \"02\",\n
                CLASS = \"0300\",\n
                CLASS_NAME = \"VGA compatible controller\",\n
                DEVICE = \"0863\",\n
                DEVICE_NAME = \"C79 [GeForce 9400M]\",\n
                DOMAIN = \"0000\",\n
                FUNCTION = \"0\",\n
                SHORT_ADDRESS = \"02:00.0\",\n
                SLOT = \"00\",\n
                TYPE = \"10de:0863:0300\",\n
                VENDOR = \"10de\",\n
                VENDOR_NAME = \"NVIDIA Corporation\"\n
            ]\n
            PCI = [
                ADDRESS = \"0000:00:06:0\",\n
                BUS = \"00\",\n
                CLASS = \"0c03\",\n
                CLASS_NAME = \"USB controller\",\n
                DEVICE = \"0aa7\",\n
                DEVICE_NAME = \"MCP79 OHCI USB 1.1 Controller\",\n
                DOMAIN = \"0000\",\n
                FUNCTION = \"0\",\n
                SHORT_ADDRESS = \"00:06.0\",\n
                SLOT = \"06\",\n
                TYPE = \"10de:0aa7:0c03\",\n
                VENDOR = \"10de\",\n
                VENDOR_NAME = \"NVIDIA Corporation\"\n
            ]\n
            PCI = [
                ADDRESS = \"0000:00:06:1\",\n
                BUS = \"00\",\n
                CLASS = \"0c03\",\n
                CLASS_NAME = \"USB controller\",\n
                DEVICE = \"0aa9\",\n
                DEVICE_NAME = \"MCP79 EHCI USB 2.0 Controller\",\n
                DOMAIN = \"0000\",\n
                FUNCTION = \"1\",\n
                SHORT_ADDRESS = \"00:06.1\",\n
                SLOT = \"06\",\n
                TYPE = \"10de:0aa9:0c03\",\n
                VENDOR = \"10de\",\n
                VENDOR_NAME = \"NVIDIA Corporation\"\n
            ]\n"
        
        results <<"\n
            HUGEPAGE = [ NODE_ID = \"0\", SIZE = \"1048576\", PAGES = \"0\", FREE = \"0\" ]\n
            HUGEPAGE = [ NODE_ID = \"0\", SIZE = \"2048\", PAGES = \"0\", FREE = \"0\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"0\", CPUS = \"0,28\" ]
            CORE = [ NODE_ID = \"0\", ID = \"1\", CPUS = \"1,29\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"2\", CPUS = \"2,30\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"3\", CPUS = \"3,31\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"4\", CPUS = \"4,32\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"5\", CPUS = \"5,33\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"6\", CPUS = \"6,34\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"8\", CPUS = \"7,35\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"9\", CPUS = \"8,36\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"10\", CPUS = \"9,37\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"11\", CPUS = \"10,38\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"12\", CPUS = \"11,39\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"13\", CPUS = \"12,40\" ]\n
            CORE = [ NODE_ID = \"0\", ID = \"14\", CPUS = \"13,41\" ]\n
            MEMORY_NODE = [ NODE_ID = \"0\", TOTAL = \"8388608\", FREE = \"#{rand(8388608)}\", USED = \"#{rand(8388608)}\", DISTANCE = \"0 1\" ]\n
            HUGEPAGE = [ NODE_ID = \"1\", SIZE = \"1048576\", PAGES = \"0\", FREE = \"0\" ]\n
            HUGEPAGE = [ NODE_ID = \"1\", SIZE = \"2048\", PAGES = \"0\", FREE = \"0\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"0\", CPUS = \"14,42\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"1\", CPUS = \"15,43\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"2\", CPUS = \"16,44\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"3\", CPUS = \"17,45\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"4\", CPUS = \"18,46\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"5\", CPUS = \"19,47\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"6\", CPUS = \"20,48\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"8\", CPUS = \"21,49\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"9\", CPUS = \"22,50\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"10\", CPUS = \"23,51\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"11\", CPUS = \"24,52\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"12\", CPUS = \"25,53\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"13\", CPUS = \"26,54\" ]\n
            CORE = [ NODE_ID = \"1\", ID = \"14\", CPUS = \"27,55\" ]\n
            MEMORY_NODE = [ NODE_ID = \"1\", TOTAL = \"8388608\", FREE = \"#{rand(8388608)}\", USED = \"#{rand(8388608)}\", DISTANCE = \"1 0\" ]\n"

        results = Base64::encode64(results).strip.delete("\n")

        send_message("MONITOR", RESULT[:success], number, results)
    end

    def stop_monitor(number, host)
        send_message("STOPMONITOR", RESULT[:success], number, nil)
    end
end


# Information Manager main program


im = DummyInformationManager.new(15)
im.start_driver

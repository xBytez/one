/* -------------------------------------------------------------------------- */
/* Copyright 2002-2019, OpenNebula Project, OpenNebula Systems                */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include "HookManager.h"
#include "NebulaLog.h"

const char * HookManager::hook_driver_name = "hook_exe";

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookManager::load_mads(int uid)
{
    HookManagerDriver *     hm_mad;
    ostringstream           oss;
    const VectorAttribute * vattr = 0;
    int                     rc;

    NebulaLog::log("HKM",Log::INFO,"Loading Hook Manager driver.");

    if ( mad_conf.size() > 0 )
    {
        vattr = static_cast<const VectorAttribute *>(mad_conf[0]);
    }

    if ( vattr == 0 )
    {
        NebulaLog::log("HKM",Log::INFO,"Failed to load Hook Manager driver.");
        return -1;
    }

    VectorAttribute hook_conf("HOOK_MAD",vattr->value());

    hook_conf.replace("NAME",hook_driver_name);

    hm_mad = new HookManagerDriver(0,hook_conf.value(),false);

    rc = add(hm_mad);

    if ( rc == 0 )
    {
        oss.str("");
        oss << "\tHook Manager loaded";

        NebulaLog::log("HKM",Log::INFO,oss);
    }

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

string * HookManager::format_message(const string& method_name, bool success,
                                     ParamList paramList, int resource_id, const string& response)
{
    ostringstream oss;

    oss << "<HOOK_MESSAGE>"
        << "<HOOK_TYPE>API</HOOK_TYPE>"
        << "<CALL>" << method_name << "</CALL>"
        << "<CALL_INFO>"
        << "<RESULT>" << success << "</RESULT>"
        << "<PARAMETERS>";

        for (int i = 0; i < paramList.size(); i++)
        {
            oss << "<PARAMETER" << i + 1 << ">"
                << paramList.get_value_as_string(i)
                << "</PARAMETER" << i + 1 << ">";
        }

    oss << "</PARAMETERS>"
        << response
        << "</CALL_INFO>"
        << "</HOOK_MESSAGE>";

    return one_util::base64_encode(oss.str());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void HookManager::send_message(const string& method_name, bool success,
                                   ParamList paramList, int resource_id, const string& response)
{
    string * message;
    const HookManagerDriver* hmd;

    hmd = get();

    message = format_message(method_name, success, paramList, resource_id, response);

    hmd->execute(*message);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

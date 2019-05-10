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

#include "HookAPI.h"
#include "NebulaLog.h"

int HookAPI::insert(SqlDB *db, string& error_str)
{
    string type_str;

    obj_template->get("NAME", name);
    obj_template->erase("NAME");

    if (name.empty())
    {
        goto error_name;
    }

    erase_template_attribute("COMMAND", cmd);

    if (cmd.empty())
    {
        goto error_cmd;
    }

    add_template_attribute("COMMAND", cmd);

    erase_template_attribute("TYPE", type_str);

    if (type_str.empty())
    {
        goto error_type;
    }
    else
    {
        type = Hook::str_to_hook_type(type_str);

        if (type == Hook::UNDEFINED)
        {
            goto error_type;
        }
    }

    erase_template_attribute("CALL", call);

    if (call.empty())
    {
        goto error_call;
    }

    add_template_attribute("CALL", call);

    return insert_replace(db, false, error_str);

error_call:
    error_str = "No CALL in template for API type Hook";
    goto error_common;
error_type:
    error_str = "No TYPE or invalid one in template for Hook.";
    goto error_common;
error_cmd:
    error_str = "No COMMAND in template for Hook.";
    goto error_common;
error_name:
    error_str = "No NAME in template for Hook.";
error_common:
    NebulaLog::log("HKM", Log::ERROR, error_str);
    return -1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int HookAPI::from_xml(const string &xml_str)
{
    int rc;

    rc = Hook::from_xml(xml_str);

    if (rc != 0)
    {
        return -1;
    }

    get_template_attribute("CALL", call);

    if (call.empty())
    {
        return -1;
    }

    return 0;
}
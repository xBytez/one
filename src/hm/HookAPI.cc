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
#include "Nebula.h"
#include "NebulaLog.h"
#include "Client.h"

int HookAPI::check_insert(Template * tmpl, string& error_str)
{
    ostringstream oss;

    tmpl->get("CALL",call);
    tmpl->erase("CALL");

    if (call.empty() || !check_api_call(call))
    {
        oss <<  "Invalid CALL \"" << call << "\" in template for API type Hook";
        error_str = oss.str();

        return -1;
    }

    tmpl->add("CALL", call);

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int HookAPI::from_template(const Template * tmpl)
{

    tmpl->get("CALL", call);

    if (call.empty() || !check_api_call(call))
    {
        return -1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookAPI::post_update_template(Template * tmpl, string& error)
{
    string new_call;

    tmpl->get("CALL", new_call);

    if (!new_call.empty() && check_api_call(new_call))
    {
        call = new_call;
    }

    tmpl->replace("CALL", call);

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool HookAPI::check_api_call(const string& api_call)
{
    RequestManager * rm = Nebula::instance().get_rm();

    return rm->exist_method(api_call);
}

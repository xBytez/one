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

#include "HookStateHost.h"
#include "NebulaLog.h"

int HookStateHost::check_insert(Template * tmpl, string& error_str)
{
    string state;
    ostringstream oss;

    //Chec STATE attribute
    tmpl->get("STATE", state);
    tmpl->erase("STATE");

    hook_state = str_to_state(state);

    if (hook_state == NONE)
    {
        oss << "Invalid STATE \"" << state << "\" in template for STATE type Hook";
        error_str = oss.str();

        return -1;
    }

    tmpl->add("STATE", state_to_str(hook_state));

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int HookStateHost::from_template(const Template * tmpl)
{
    string state_str;

    tmpl->get("STATE", state_str);

    hook_state = str_to_state(state_str);

    if (hook_state == NONE)
    {
        return -1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookStateHost::post_update_template(Template * tmpl, string& error)
{
    string new_state, resource;

    tmpl->get("STATE", new_state);

    if (str_to_state(new_state) == NONE)
    {
        new_state = state_to_str(hook_state);
    }

    tmpl->replace("STATE", new_state);

    tmpl->get("RESOURCE", resource);

    if (PoolObjectSQL::str_to_type(resource) != PoolObjectSQL::HOST)
    {
        resource = PoolObjectSQL::type_to_str(PoolObjectSQL::HOST);
    }

    tmpl->replace("RESOURCE", resource);

    return 0;
}

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

#include "HookStateVM.h"
#include "NebulaLog.h"
#include "VirtualMachine.h"

int HookStateVM::check_insert(Template * tmpl, string& error_str)
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

    //Check CUSTOM states if needed
    if (hook_state == CUSTOM)
    {
        string vm_state, vm_lcm_state;

        tmpl->get("CUSTOM_STATE", vm_state);
        tmpl->erase("CUSTOM_STATE");

        if (VirtualMachine::vm_state_from_str(vm_state, custom_state) == -1)
        {
            oss << "Invalid CUSTOM_STATE \"" << vm_state << "\" in template for STATE type Hook";
            error_str = oss.str();

            return -1;
        }

        tmpl->add("CUSTOM_STATE", vm_state);

        tmpl->get("CUSTOM_LCM_STATE", vm_lcm_state);
        tmpl->erase("CUSTOM_LCM_STATE");

        if (VirtualMachine::lcm_state_from_str(vm_lcm_state, custom_lcm_state) == -1)
        {
            oss << "Invalid CUSTOM_LCM_STATE \"" << vm_lcm_state << "\" in template for STATE type Hook";
            error_str = oss.str();

            return -1;
        }

        tmpl->add("CUSTOM_LCM_STATE", vm_lcm_state);
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int HookStateVM::from_template(const Template * tmpl, string& error)
{
    string state_str;

    tmpl->get("STATE", state_str);

    hook_state = str_to_state(state_str);

    if (hook_state == NONE)
    {
        ostringstream oss;

        oss << "Invalid STATE: " << state_str;
        error = oss.str();

        return -1;
    }

    if (hook_state == CUSTOM)
    {
        string vm_state_str, lcm_state_str;

        tmpl->get("CUSTOM_STATE", vm_state_str);

        if (VirtualMachine::vm_state_from_str(vm_state_str, custom_state) == -1)
        {
            ostringstream oss;

            oss << "Invalid CUSTOM_STATE: " << custom_state;
            error = oss.str();

            return -1;
        }

        tmpl->get("CUSTOM_LCM_STATE", lcm_state_str);

        if (VirtualMachine::lcm_state_from_str(lcm_state_str, custom_lcm_state) == -1)
        {
            ostringstream oss;

            oss << "Invalid CUSTOM_LCM_STATE: " << lcm_state_str;
            error = oss.str();

            return -1;
        }
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookStateVM::post_update_template(Template * tmpl, string& error)
{
    string new_state, resource;

    tmpl->get("STATE", new_state);

    if (str_to_state(new_state) == NONE)
    {
        new_state = state_to_str(hook_state);
    }

    tmpl->replace("STATE", new_state);

    tmpl->get("RESOURCE", resource);

    if (PoolObjectSQL::str_to_type(resource) != PoolObjectSQL::VM)
    {
        resource = PoolObjectSQL::type_to_str(PoolObjectSQL::VM);
    }

    tmpl->replace("RESOURCE", resource);

    if (new_state == state_to_str(CUSTOM))
    {
        VirtualMachine::VmState vm_state;
        VirtualMachine::LcmState lcm_state;

        tmpl->get("CUSTOM_STATE", new_state);

        if (VirtualMachine::vm_state_from_str(new_state, vm_state) == -1)
        {
            VirtualMachine::vm_state_to_str(new_state, custom_state);
        }

        tmpl->replace("CUSTOM_STATE", new_state);

        tmpl->get("CUSTOM_LCM_STATE", new_state);

        if (VirtualMachine::lcm_state_from_str(new_state, lcm_state) == -1)
        {
            VirtualMachine::lcm_state_to_str(new_state, custom_lcm_state);
        }

        tmpl->replace("CUSTOM_LCM_STATE", new_state);
    }

    return 0;
}

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

#ifndef HOOK_STATE_VM_H_
#define HOOK_STATE_VM_H_

#include <string>

#include "Hook.h"
#include "VirtualMachine.h"
#include "HookImplementation.h"

class HookStateVM : public HookImplementation
{
public:

    enum HookVMStates
    {
        CREATE   = 0,
        RUNNING  = 1,
        SHUTDOWN = 2,
        STOP     = 3,
        DONE     = 4,
        UNKNOWN  = 5,
        CUSTOM   = 6,
        NONE     = 7
    };

    static string state_to_str(HookVMStates st)
    {
        switch(st)
        {
            case CREATE:   return "CREATE";   break;
            case RUNNING:  return "RUNNING";  break;
            case SHUTDOWN: return "SHUTDOWN"; break;
            case STOP:     return "STOP";     break;
            case DONE:     return "DONE";     break;
            case UNKNOWN:  return "UNKNOWN";  break;
            case CUSTOM:   return "CUSTOM";   break;
            default:       return "";
        }
    };

    static HookVMStates str_to_state(string st)
    {
        if ( st == "CREATE" )        return CREATE;
        else if ( st == "RUNNING" )  return RUNNING;
        else if ( st == "SHUTDOWN" ) return SHUTDOWN;
        else if ( st == "STOP" )     return STOP;
        else if ( st == "DONE" )     return DONE;
        else if ( st == "UNKNOWN" )  return UNKNOWN;
        else if ( st == "CUSTOM" )   return CUSTOM;
        else                         return NONE;
    };

    void do_hook(void *arg)
    {
        return;
    }

private:
    friend class HookPool;
    friend class Hook;

    // *************************************************************************
    // Constructor/Destructor
    // *************************************************************************

    HookStateVM():hook_state(NONE){};

    HookStateVM(const HookVMStates state): hook_state(state){};

    HookStateVM(const HookVMStates state, const VirtualMachine::VmState vm_state,
                const VirtualMachine::LcmState vm_lcm_state):
    hook_state(state),
    custom_state(vm_state),
    custom_lcm_state(vm_lcm_state){};

    ~HookStateVM(){};

    /**
     *  Check if type dependent attributes are well defined.
     *    @param tmpl pointer to the Hook template
     *    @param error_str string with error information
     *    @return 0 on success
     */
    int check_insert(Template *tmpl, string& error_str);

    /**
     *  Rebuilds the object from a template
     *    @param tmpl The template
     *
     *    @return 0 on success, -1 otherwise
     */
    int from_template(const Template * tmpl);

    /* Checks the mandatory template attributes
     *    @param tmpl The hook template
     *    @param error string describing the error if any
     *    @return 0 on success
     */
    int post_update_template(Template * tmpl, string& error);

    // -------------------------------------------------------------------------
    // Hook API Attributes
    // -------------------------------------------------------------------------

    /**
     *  States hook_state state which trigger the hook
     */
    HookVMStates hook_state;

    /**
     * VirtualMachine::VmState custom_state VM state which trigger the hook
     * if hook_state is set to CUSTOM
     */
    VirtualMachine::VmState custom_state;

    /**
     * VirtualMachine::LcmState custom_lcm_state VM LCM state which trigger the hook
     * if hook_state is set to CUSTOM
     */
    VirtualMachine::LcmState custom_lcm_state;

};

#endif
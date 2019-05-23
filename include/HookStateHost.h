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

#ifndef HOOK_STATE_HOST_H_
#define HOOK_STATE_HOST_H_

#include <string>

#include "Hook.h"
#include "HookImplementation.h"

class HookStateHost : public HookImplementation
{
public:

    enum HookHostStates
    {
        CREATE  = 0,
        ERROR   = 1,
        DISABLE = 2,
        ENABLE  = 3,
        OFFLINE = 4,
        NONE    = 5
    };

    static string state_to_str(HookHostStates st)
    {
        switch(st)
        {
            case CREATE:  return "CREATE";  break;
            case ERROR:   return "ERROR";   break;
            case DISABLE: return "DISABLE"; break;
            case ENABLE:  return "ENABLE";  break;
            case OFFLINE: return "OFFLINE"; break;
            default:      return "";
        };
    };

    static HookHostStates str_to_state(string st)
    {
        if ( st == "CREATE" )       return CREATE;
        else if ( st == "ERROR" )   return ERROR;
        else if ( st == "DISABLE" ) return DISABLE;
        else if ( st == "ENABLE" )  return ENABLE;
        else if ( st == "OFFLINE" ) return OFFLINE;
        else                        return NONE;
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

    HookStateHost():hook_state(NONE){};

    HookStateHost(const HookHostStates state): hook_state(state){};

    ~HookStateHost(){};

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
    HookHostStates hook_state;

};

#endif
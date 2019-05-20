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

#ifndef HOOK_API_H_
#define HOOK_API_H_

#include <string>

#include "Hook.h"
#include "HookImplementation.h"

class HookAPI : public HookImplementation
{
public:

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

    HookAPI():call(""){};

    HookAPI(const Template * tmpl)
    {
        tmpl->get("CALL",call);
    };

    ~HookAPI(){};

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
     *  String representation of the API call
     */
    string call;
};

#endif
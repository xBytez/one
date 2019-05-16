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

using namespace std;

class HookAPI : public Hook
{
public:
    void do_hook(void *arg)
    {
        return;
    }
private:
    friend class HookPool;
    // *************************************************************************
    // Constructor/Destructor
    // *************************************************************************

    HookAPI(Template * tmpl):
        Hook(tmpl),
        call("")
    {
        if (tmpl != 0)
        {
            obj_template = tmpl;
        }
        else
        {
            obj_template = new Template();
        }
    };

    HookAPI(Hook * hook):
    Hook(0),
    call("")
    {
        //Implement
    };

    virtual ~HookAPI(){};

    /**
     *  Writes the Hook in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int insert(SqlDB *db, string& error_str);

    /**
     *  Rebuilds the object from an xml formatted string
     *    @param xml_str The xml-formatted string
     *
     *    @return 0 on success, -1 otherwise
     */
    int from_xml(const string &xml_str);

    /* Checks the mandatory template attributes
     *    @param error string describing the error if any
     *    @return 0 on success
     */
    int post_update_template(string& error);

    // -------------------------------------------------------------------------
    // Hook API Attributes
    // -------------------------------------------------------------------------

    /**
     *  String representation of the API call
     */
    string call;
};

#endif
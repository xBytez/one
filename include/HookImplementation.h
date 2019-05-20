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

#ifndef HOOK_IMPLEMENTATION_H_
#define HOOK_IMPLEMENTATION_H_

#include <string>
#include "Hook.h"
#include "Template.h"

class HookImplementation
{
private:
    friend class Hook;

    virtual int from_template(const Template * tmpl) = 0;

    virtual int post_update_template(Template * tmpl, string& error) = 0;

    virtual int check_insert(Template *tmpl, string& error_str) = 0;
};

#endif
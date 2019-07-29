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

#ifndef HOOK_MANAGER_H_
#define HOOK_MANAGER_H_

#include "MadManager.h"
#include "ActionManager.h"
#include "HookManagerDriver.h"
#include "VirtualMachinePool.h"
#include "Request.h"

using namespace std;

class HookManager : public MadManager
{
public:

    HookManager(vector<const VectorAttribute*>& _mads, VirtualMachinePool * _vmpool)
        :MadManager(_mads){};

    ~HookManager(){};

    /**
     *  Loads Hook Manager Mads defined in configuration file
     *   @param uid of the user executing the driver. When uid is 0 the nebula
     *   identity will be used. Otherwise the Mad will be loaded through the
     *   sudo application.
     */
    int load_mads(int uid=0);

    /**
     *  Returns a pointer to a Information Manager MAD. The driver is
     *  searched by its name and owned by oneadmin with uid=0.
     *    @param name of the driver
     *    @return the Hook driver owned by uid 0, with attribute "NAME" equal to
     *    name or 0 in not found
     */
    const HookManagerDriver * get()
    {
        string name("NAME");

        return static_cast<const HookManagerDriver *>
               (MadManager::get(0,name,hook_driver_name));
    };

    string * format_message(const string& method_name, bool success,
                            ParamList paramList, int resource_id, const string& response);

    void send_message(const string& method_name, bool success,
                          ParamList paramList, int resource_id, const string& response);

private:
    /**
     *  Generic name for the Hook driver
     */
     static const char *  hook_driver_name;
};

#endif /*HOOK_MANAGER_H*/


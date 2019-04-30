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

#ifndef HOOK_H_
#define HOOK_H_

#include <string>

#include "PoolObjectSQL.h"

using namespace std;

class Hook : public PoolObjectSQL
{
public:

    /**
     *  Defines the hook type, so a whole hook class can be masked
     */
    enum HookType
    {
        ALLOCATE = 0x01,
        UPDATE   = 0x02,
        REMOVE   = 0x04,
        STATE    = 0x08,
        API      = 0x10
    };

    /**
     *  Executes the hook it self (usually with the aid of the ExecutionManager)
     *    @param arg additional arguments for the hook
     */
    virtual void do_hook(void *arg) = 0;

private:

    // *************************************************************************
    // Constructor/Destructor
    // *************************************************************************

    Hook(
        int id,
        const string &_name,
        const string &_cmd,
        const string &_args,
        HookType  _ht,
        PoolObjectSQL::ObjectType  _rt,
        bool _remote):
            PoolObjectSQL(id,HOOK,_name,-1,-1,"","",table),
            type(_ht),
            resource_type(_rt),
            cmd(_cmd),
            args(_args),
            remote(_remote){};

    virtual ~Hook();

    /**
     *  Parses the arguments of the hook using a generic $ID identifier, and
     *  the target object.  $TEMPLATE will be the base64 encoding of the
     *  template and $ID the oid of the object.
     *    @param obj pointer to the object executing the hook for
     *    @param the resulting parser arguments
     */
    void parse_hook_arguments(PoolObjectSQL * obj,
                              string&         parsed);

    /**
     * Function to print the Hook object into a string in XML format
     *  @param xml the resulting XML string
     *  @return a reference to the generated string
     */
    string& to_xml(string& xml) const;

    /**
     *  Rebuilds the object from an xml formatted string
     *    @param xml_str The xml-formatted string
     *
     *    @return 0 on success, -1 otherwise
     */
    int from_xml(const string &xml_str);


    // *************************************************************************
    // Database implementation
    // *************************************************************************

    static const char * db_names;

    static const char * db_bootstrap;

    static const char * table;

    /**
     *  Execute an INSERT or REPLACE Sql query.
     *    @param db The SQL DB
     *    @param replace Execute an INSERT or a REPLACE
     *    @param error_str Returns the error reason, if any
     *    @return 0 one success
     */
    int insert_replace(SqlDB *db, bool replace, string& error_str);

    /**
     *  Bootstraps the database table(s) associated to the Host
     *    @return 0 on success
     */
    static int bootstrap(SqlDB * db)
    {
        int rc;

        ostringstream oss_hook(Hook::db_bootstrap);

        rc =  db->exec_local_wr(oss_hook);

        return rc;
    };

    /**
     *  Writes the Host and its associated HostShares in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int insert(SqlDB *db, string& error_str)
    {
        return insert_replace(db, false, error_str);
    };

    /**
     *  Writes/updates the Hosts data fields in the database.
     *    @param db pointer to the db
     *    @return 0 on success
     */
    int update(SqlDB *db)
    {
        string error_str;
        return insert_replace(db, true, error_str);
    };

    // -------------------------------------------------------------------------
    // Hook Attributes
    // -------------------------------------------------------------------------

    /**
     * The hook type
     */
    HookType type;

    /**
     * The type of the resource which change trigger the hook
     */
    PoolObjectSQL::ObjectType resource_type;

    /**
     *  The command to be executed
     */
    string   cmd;

    /**
     *  The arguments for the command
     */
    string   args;

    /**
     *  True if the command is to be executed remotely
     */
    bool     remote;

};

#endif /*HOOK_H_*/

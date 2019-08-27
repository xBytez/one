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

#ifndef HOOKLOG_H_
#define HOOKLOG_H_

#include <string>

#include "ActionManager.h"

class SqlDB;

/**
 *  Thread loop (timer action to purge the log)
 */
extern "C" void * hlog_action_loop(void *arg);

/**
 *  This class represents the execution log of Hooks. It writes/reads execution
 *  records in the DB.
 */
class HookLog: public ActionListener
{
public:

    HookLog(SqlDB *db);

    virtual ~HookLog() = default;

    /**
     *  Get the execution log for a given hook
     *    @param hkid the ID of the hook
     *    @param xml_log execution records in XML format
     *
     *    @return 0 on success
     */
    int dump_log(int hkid, std::string &xml_log);

    /**
     *  Get the execution log for all hooks
     *    @param xml_log execution records in XML format
     *
     *    @return 0 on success
     */
    int dump_log(std::string &xml_log);

    /**
     *  Adds a new execution record to the hook
     *    @param hkid the ID of the hook
     *    @param rc return code of the execution
     *    @param xml_result rc, std streams and execution context
     *
     *    @return 0 on success
     */
    int add(int hkid, int rc, std::string &xml_result);

    /**
     *  Retries a given execution for a host, using the same execution context
     *    @param hkid the ID of the hook
     *    @param exeid the execution identifier
     *    @param err_msg error message
     *
     *    @return 0 on success
     */
    int retry(int hkid, int exeid, std::string& err_msg);

    /**
     *  This functions starts the associated listener thread, and creates a
     *  new thread for the Hook Manager. This thread will wait in
     *  an action loop till it receives ACTION_FINALIZE.
     *    @return 0 on success.
     */
    int start();

    /**
     *  Gets the HookManager thread identification.
     *    @return pthread_t for the manager thread (that in the action loop).
     */
    pthread_t get_thread_id() const
    {
        return hm_thread;
    };

    /**
     *  Terminates the hook manager thread listener
     */
    void finalize()
    {
        am.finalize();
    };

    /**
     *  Bootstraps the database table(s) associated to the Hook Log
     *    @return 0 on success
     */
    static int bootstrap(SqlDB *_db);

private:
    /**
     *  Function to execute the Manager action loop method within a new pthread
     *  (requires C linkage)
     */
    friend void * hlog_action_loop(void *arg);

    // ----------------------------------------
    // DataBase implementation variables
    // ----------------------------------------
    static const char * table;

    static const char * db_names;

    static const char * db_bootstrap;

    /**
     *  Pointer to the database.
     */
    SqlDB * db;

    /**
     *  Thread id for the HookLog class
     */
    pthread_t             hm_thread;

    /**
     *  Action engine for the Manager
     */
    ActionManager         am;

    /**
     *  Dumps hook log records
     *    @param hkid -1 to dump all records
     *    @param exec_id -1 to dump all records
     *    @param xml_log execution results in xml format
     *
     *    @return 0 on success
     */
    int _dump_log(int hkid, int exec_id, std::string &xml_log);

    // -------------------------------------------------------------------------
    // Action Listener interface
    // -------------------------------------------------------------------------
    /**
     *  This function is executed periodically to purge the Hook Log
     */
    void timer_action(const ActionRequest& ar);

    void finalize_action(const ActionRequest& ar);
};

#endif /*HOOKLOG_H_*/

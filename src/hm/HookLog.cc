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

#include "HookLog.h"
#include "ObjectXML.h"
#include "NebulaLog.h"
#include "SqlDB.h"

#include <sstream>

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

const char * HookLog::table = "hook_log";

const char * HookLog::db_names = "hkid, exeid, timestamp, rc, body";

const char * HookLog::db_bootstrap = "CREATE TABLE IF NOT EXISTS hook_log"
    " (hkid INTEGER, exeid INTEGER, timestamp INTEGER, rc INTEGER,"
    " body MEDIUMTEXT,PRIMARY KEY(hkid, exeid))";

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookLog::bootstrap(SqlDB * db)
{
    std::ostringstream oss_hook(HookLog::db_bootstrap);

    return db->exec_local_wr(oss_hook);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

HookLog::HookLog(SqlDB *_db):db(_db){};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

extern "C" void * hlog_action_loop(void *arg)
{
    HookLog * hlog;

    if ( arg == 0 )
    {
        return 0;
    }

    NebulaLog::log("HKM", Log::INFO, "HookLog Manager started.");

    hlog = static_cast<HookLog *>(arg);

    hlog->am.loop();

    NebulaLog::log("HKM", Log::INFO, "HookLog Manager stopped.");

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookLog::start()
{
    pthread_attr_t pattr;

    NebulaLog::log("HKM", Log::INFO, "Starting HookLog Manager...");

    pthread_attr_init (&pattr);
    pthread_attr_setdetachstate (&pattr, PTHREAD_CREATE_JOINABLE);

    return pthread_create(&hm_thread, &pattr, hlog_action_loop, (void *) this);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookLog::_dump_log(int hkid, std::string &xml_log)
{
    std::ostringstream cmd;

    string_cb cb(1);

    cmd << "SELECT body FROM "<< table;

    if ( hkid == -1 )
    {
        cmd << "ORDER BY hkid DESC";
    }
    else
    {
        cmd << "WHERE hkid = " << hkid;
    }

    xml_log.append("<HOOKLOG>");

    cb.set_callback(&xml_log);

    int rc = db->exec_rd(cmd, &cb);

    cb.unset_callback();

    xml_log.append("</HOOKLOG>");

    return rc;
}

/* -------------------------------------------------------------------------- */

int HookLog::dump_log(int hkid, std::string &xml_log)
{
    return _dump_log(hkid, xml_log);
}

/* -------------------------------------------------------------------------- */

int HookLog::dump_log(std::string &xml_log)
{
    return _dump_log( -1, xml_log);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookLog::add(int hkid, int hkrc, std::string &xml_result)
{
    std::ostringstream oss;

    single_cb<int> cb;

    int last_exeid = -1;

    char * sql_xml;

    std::string xml_body;

    cb.set_callback(&last_exeid);

    oss << "SELECT IFNULL(MAX(exeid), -1) FROM hook_log" << " WHERE hkid = " << hkid;

    int rc = db->exec_rd(oss, &cb);

    if ( rc != 0 )
    {
        return rc;
    }

    last_exeid++;

    cb.unset_callback();

    oss.str("");

    time_t the_time = time(0);

    oss << "<HOOK_EXECUTION_RECORD>"
        << "<HOOK_ID>" << hkid << "</HOOK_ID>"
        << "<EXECUTION_ID>" << last_exeid << "</EXECUTION_ID>"
        << "<TIMESTAMP>" << the_time << "</TIMESTAMP>"
        << xml_result
        << "</HOOK_EXECUTION_RECORD>";

    sql_xml = db->escape_str(oss.str().c_str());

    if ( sql_xml == 0 )
    {
        return -1;
    }

    if ( ObjectXML::validate_xml(sql_xml) != 0 )
    {
        return -1;
    }

    oss.str("");

    oss <<"INSERT INTO "<< table <<" ("<< db_names <<") VALUES ("
        << hkid       << ","
        << last_exeid << ","
        << the_time   << ","
        << hkrc       << ","
        << "'" << sql_xml << "')";

    rc = db->exec_wr(oss);

    db->free_str(sql_xml);

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int HookLog::retry(int hkid, int exeid)
{
    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void HookLog::timer_action(const ActionRequest& ar)
{
    //PURGE
    return;
}

void HookLog::finalize_action(const ActionRequest& ar)
{
    NebulaLog::log("HKM",Log::INFO,"Stopping HookLog Manager...");
};


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

#include "Hook.h"
#include "Nebula.h"
#include "string"

/* ************************************************************************ */
/* Hook :: Database Access Functions                                        */
/* ************************************************************************ */

const char * Hook::table = "hook_pool";

const char * Hook::db_names =
    "oid, name, body, uid, gid, owner_u, group_u, other_u, type";

const char * Hook::db_bootstrap = "CREATE TABLE IF NOT EXISTS hook_pool ("
    "oid INTEGER PRIMARY KEY, name VARCHAR(128), body MEDIUMTEXT, uid INTEGER,"
    "gid INTEGER, owner_u INTEGER, group_u INTEGER, other_u INTEGER, type INTEGER)";

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*
void AllocateRemoveHook::do_hook(void *arg)
{
    PoolObjectSQL * obj = static_cast<PoolObjectSQL *>(arg);
    string          parsed_args = args;

    if ( obj == 0 )
    {
        return;
    }

    parse_hook_arguments(obj, parsed_args);

    Nebula& ne                    = Nebula::instance();
    HookManager * hm              = ne.get_hm();
    const HookManagerDriver * hmd = hm->get();

    if ( hmd != 0 )
    {
        if ( remote == true )
        {
            string hostname;

            hmd->execute(obj->get_oid(),
                         name,
                         remote_host(obj, hostname),
                         cmd,
                         parsed_args);
        }
        else
        {
            hmd->execute(obj->get_oid(), name, cmd, parsed_args);
        }
    }
}*/

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void Hook::parse_hook_arguments(PoolObjectSQL *obj,
                                string&       parsed)
{
    size_t  found;

    found = parsed.find("$ID");

    if ( found !=string::npos )
    {
        ostringstream oss;
        oss << obj->get_oid();

        parsed.replace(found, 3, oss.str());
    }

    found = parsed.find("$TEMPLATE");

    if ( found != string::npos )
    {
        string templ;
        parsed.replace(found, 9, obj->to_xml64(templ));
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

string& Hook::to_xml(string& xml) const
{
    string template_xml;

    ostringstream oss;

    oss <<
    "<HOOK>"
        "<ID>"     << oid    << "</ID>"     <<
        "<NAME>"   << name   << "</NAME>"   <<
        "<TYPE>"   << type   << "</TYPE>"   <<
        obj_template->to_xml(template_xml)  <<
    "</HOOK>";

    xml = oss.str();

    return xml;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int Hook::from_xml(const string& xml)
{
    vector<xmlNodePtr> content;
    string type_str;

    int rc = 0;

    // Initialize the internal XML object
    update_from_str(xml);

    // Get class base attributes
    rc += xpath(oid,        "/HOOK/ID", -1);
    rc += xpath(name,       "/HOOK/NAME",   "not_found");
    rc += xpath(type_str,   "/HOOK/TYPE",   "");

    type = str_to_hook_type(type_str);

    // Set the owner and group to oneadmin
    set_user(0, "");
    set_group(GroupPool::ONEADMIN_ID, GroupPool::ONEADMIN_NAME);

    // ------------ Template ---------------

    ObjectXML::get_nodes("/HOOK/TEMPLATE", content);

    if( content.empty())
    {
        return -1;
    }

    rc += obj_template->from_xml_node( content[0] );

    ObjectXML::free_nodes(content);

    content.clear();

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int Hook::insert_replace(SqlDB *db, bool replace, string& error_str){
    ostringstream   oss;

    int    rc;
    string xml_body;

    char * sql_name;
    char * sql_xml;

    // Set the owner and group to oneadmin
    set_user(0, "");
    set_group(GroupPool::ONEADMIN_ID, GroupPool::ONEADMIN_NAME);

   // Update the Host

    sql_name = db->escape_str(name.c_str());

    if ( sql_name == 0 )
    {
        goto error_name;
    }

    sql_xml = db->escape_str(to_xml(xml_body).c_str());

    if ( sql_xml == 0 )
    {
        goto error_body;
    }

    if ( validate_xml(sql_xml) != 0 )
    {
        goto error_xml;
    }

    if(replace)
    {
        oss << "REPLACE";
    }
    else
    {
        oss << "INSERT";
    }

    // Construct the SQL statement to Insert or Replace
    oss <<" INTO "<<table <<" ("<< db_names <<") VALUES ("
        <<          oid            << ","
        << "'" <<   sql_name       << "',"
        << "'" <<   sql_xml        << "',"
        <<          uid            << ","
        <<          gid            << ","
        <<          owner_u        << ","
        <<          group_u        << ","
        <<          other_u        << ","
        <<          type           << ")";

    rc = db->exec_wr(oss);

    db->free_str(sql_name);
    db->free_str(sql_xml);

    return rc;

error_xml:
    db->free_str(sql_name);
    db->free_str(sql_xml);

    error_str = "Error transforming the Hook to XML.";

    goto error_common;

error_body:
    db->free_str(sql_name);
    goto error_generic;

error_name:
    goto error_generic;

error_generic:
    error_str = "Error inserting Hook in DB.";
error_common:
    return -1;
}

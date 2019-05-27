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


#ifndef VM_XML_H_
#define VM_XML_H_

#include <sstream>

#include "ObjectXML.h"
#include "HostPoolXML.h"
#include "Resource.h"

#include "VirtualMachineTemplate.h"
#include "ScheduledAction.h"

class ImageDatastorePoolXML;

using namespace std;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

class VirtualMachineNicXML : public ObjectXML
{
public:

    /**
     *  Returns a vector of matched datastores
    */
    const vector<Resource *>& get_match_networks()
    {
        return match_networks.get_resources();
    }

    /**
     *  Adds a matching network
     *    @param oid of the network
     */
    void add_match_network(int oid)
    {
        match_networks.add_resource(oid);
    }

    /**
     *  Sort the matched networks for the VM
     */
    void sort_match_networks()
    {
       match_networks.sort_resources();
    }

    /**
     *  Removes the matched networks
     */
    void clear_match_networks()
    {
        match_networks.clear();
    }

    //--------------------------------------------------------------------------
    // Rank & requirements set & get
    //--------------------------------------------------------------------------
    const string& get_rank()
    {
        return rank;
    };

    void set_rank(const string& r)
    {
        rank = r;
    }

    const string& get_requirements()
    {
        return requirements;
    };

    void set_requirements(const string& r)
    {
        requirements = r;
    }

private:
    ResourceMatch match_networks;

    string rank;

    string requirements;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

class VirtualMachineXML : public ObjectXML
{
public:

    VirtualMachineXML(const string &xml_doc): ObjectXML(xml_doc)
    {
        init_attributes();
    };

    VirtualMachineXML(const xmlNodePtr node): ObjectXML(node)
    {
        init_attributes();
    }

    ~VirtualMachineXML()
    {
        if (vm_template != 0)
        {
            delete vm_template;
        }

        if (user_template != 0)
        {
            delete user_template;
        }
    }

    //--------------------------------------------------------------------------
    // Get Methods for VirtualMachineXML class
    //--------------------------------------------------------------------------
    int oid() const { return _oid; };

    int uid() const { return _uid; };

    int gid() const { return _gid; };

    int hid() const { return _hid; };

    int dsid() const { return _dsid; };

    time_t get_stime() const { return stime; }

    bool is_resched() const { return (resched == 1); }

    bool is_resume() const { return resume; }

    bool is_public_cloud() const { return public_cloud; }

    bool is_active() const { return state == 3; }

    map<int,long long> get_storage_usage() { return ds_usage; }

    /**
     *  Returns a VirtualMachineNicXML
     */
    VirtualMachineNicXML * get_nic(int nic_id)
    {
        VirtualMachineNicXML * n = 0;

        std::map<int, VirtualMachineNicXML *>::iterator it = nics.find(nic_id);

        if ( it != nics.end() )
        {
            n = it->second;
        }

        return n;
    }

    //--------------------------------------------------------------------------
    // Scheduling requirements and rank
    //--------------------------------------------------------------------------
    const string& get_rank()
    {
        return rank;
    };

    const string& get_ds_rank()
    {
        return ds_rank;
    };

    const string& get_nic_rank(int nic_id)
    {
        static std::string es;

        std::map<int, VirtualMachineNicXML *>::iterator it = nics.find(nic_id);

        if ( it != nics.end() )
        {
            return it->second->get_rank();
        }

        return es;
    };

    const string& get_requirements()
    {
        return requirements;
    };

    const string& get_ds_requirements()
    {
        return ds_requirements;
    }

    const string& get_nic_requirements(int nic_id)
    {
        static std::string es;

        std::map<int, VirtualMachineNicXML *>::iterator it = nics.find(nic_id);

        if ( it != nics.end() )
        {
            return it->second->get_requirements();
        }

        return es;
    }

    //--------------------------------------------------------------------------
    // Capacity Interface
    //--------------------------------------------------------------------------
    /**
     *  This function fills a share capacity request based on the VM information:
     *    - cpu
     *    - memory
     *    - system_ds disk usage
     *    - PCI devices
     *    - NUMA node topology
     */
    void get_capacity(HostShareCapacity &sr);

    /**
     *  Add capacity to (cpu, mem, system_ds disk, numa_nodes) this VM
     *    @param the capacity to be added
     */
    void add_capacity(HostShareCapacity &sr);

    /**
     *  Clears the capacity allocaton of this VM and return it.
     *    @param sr, the sorage requirements
     */
    void reset_capacity(HostShareCapacity &sr);

    /**
     *  Adds (logical AND) new placement requirements to the current ones
     *    @param reqs additional requirements
     */
    void add_requirements(const string& reqs)
    {
        if ( reqs.empty() )
        {
            return;
        }
        else if ( requirements.empty() )
        {
            requirements = reqs;
        }
        else
        {
            requirements += " & (" + reqs + ")";
        }
    }

    //--------------------------------------------------------------------------
    // Matched Resources Interface
    //--------------------------------------------------------------------------
    /**
     *  Adds a matching host if it is not equal to the actual one
     *    @param oid of the host
     */
    void add_match_host(int oid)
    {
        if (( resched == 1 && _hid != oid ) || ( resched == 0 ))
        {
            match_hosts.add_resource(oid);
        }
    };

    /**
     *  Adds a matching datastore
     *    @param oid of the datastore
     */
    void add_match_datastore(int oid)
    {
        match_datastores.add_resource(oid);
    }

    /**
     *  Adds a matching network
     *    @param oid of the network
     */
    void add_match_network(int oid, int nic_id)
    {
        std::map<int, VirtualMachineNicXML *>::iterator it = nics.find(nic_id);

        if ( it != nics.end() )
        {
            it->second->add_match_network(oid);
        }
    }

    /**
     *  Returns a vector of matched hosts
     */
    const vector<Resource *> get_match_hosts()
    {
        return match_hosts.get_resources();
    }

    /**
     *  Returns a vector of matched datastores
     */
    const vector<Resource *> get_match_datastores()
    {
        return match_datastores.get_resources();
    }

    /**
     *  Returns a vector of matched networks
     */
    const vector<Resource *>& get_match_networks(int nic_id)
    {
        static std::vector<Resource *> ev;

        std::map<int, VirtualMachineNicXML *>::iterator it = nics.find(nic_id);

        if ( it != nics.end() )
        {
            return it->second->get_match_networks();
        }

        return ev;
    }

    /**
     *  Sort the matched hosts for the VM
     */
    void sort_match_hosts()
    {
        match_hosts.sort_resources();
    }

    /**
     *  Sort the matched datastores for the VM
     */
    void sort_match_datastores()
    {
        match_datastores.sort_resources();
    }

    /**
     *  Sort the matched networks for the VM
     */
    void sort_match_networks(int nic_id)
    {
        std::map<int, VirtualMachineNicXML *>::iterator it = nics.find(nic_id);

        if ( it != nics.end() )
        {
            it->second->sort_match_networks();
        }
    }

    /**
     *  Removes the matched hosts
     */
    void clear_match_hosts()
    {
        match_hosts.clear();
    }

    /**
     *  Removes the matched datastores
     */
    void clear_match_datastores()
    {
        match_datastores.clear();
    }

    /**
     *  Removes the matched networks
     */
    void clear_match_networks()
    {
        map<int, VirtualMachineNicXML *>::iterator it;

        for (it = nics.begin(); it != nics.end(); it++ )
        {
            it->second->clear_match_networks();
        }
    }

    /**
     * Marks the VM to be only deployed on public cloud hosts
     */
    void set_only_public_cloud();

    /**
     * Returns true is the VM can only be deployed in public cloud hosts
     * @return true is the VM can only be deployed in public cloud hosts
     */
    bool is_only_public_cloud() const;

    /**
     *  Add a VM to the set of affined VMs. This is used for the VM leader
     *  when scheduling a group.
     *
     *  @param vmid of the affined vm
     *
     */
    void add_affined(int vmid)
    {
        affined_vms.insert(vmid);
    }

    const set<int>& get_affined_vms() const
    {
        return affined_vms;
    }

    //--------------------------------------------------------------------------
    // Capacity Interface
    //--------------------------------------------------------------------------

    /**
     *  Tests if the Image DS have enough free space to host the VM
     *    @param img_datastores Image Datastores
     *    @param error_msg error reason
     *    @return true if the Image Datastores can host the VM
     */
    bool test_image_datastore_capacity(
            ImageDatastorePoolXML * img_dspool, string & error_msg) const;

    /**
     *  Adds the VM disk requirements to each Image Datastore counter
     *    @param img_datastores Image Datastores
     */
    void add_image_datastore_capacity(ImageDatastorePoolXML * img_dspool);

    //--------------------------------------------------------------------------
    // Action Interface
    //--------------------------------------------------------------------------

    /**
     *  Get the user template of the VM
     *    @return the template as a XML string
     */
    string& get_template(string& xml_str)
    {
        if (user_template != 0)
        {
            user_template->to_xml(xml_str);
        }
        else
        {
            xml_str = "";
        }

        return xml_str;
    }

    /**
     * Removes (but does not delete) the scheduled actions of the VM
     *
     * @param attributes to hold the VM actions
     */
    SchedActions * get_actions()
    {
        return new SchedActions(user_template);
    }

    /**
     * Sets an attribute in the VM Template, it must be allocated in the heap
     *
     * @param attributes to hold the VM actions
     */
    void set_attribute(Attribute* att)
    {
        return user_template->set(att);
    }

    /**
     *  Checks the action to be performed and returns the corresponding XML-RPC
     *  method name.
     *    @param action_st, the action to be performed. The XML-RPC name is
     *    returned here
     *    @return 0 on success.
     */
    static int parse_action_name(string& action_st);

    /**
     *  Function to write a Virtual Machine in an output stream
     */
    friend ostream& operator<<(ostream& os, VirtualMachineXML& vm);

    /**
     * Adds a message to the VM's USER_TEMPLATE/SCHED_MESSAGE attribute
     *   @param st Message to set
     */
    void log(const string &st);

    /**
     * Clears the VM's USER_TEMPLATE/SCHED_MESSAGE attribute
     * @return true if the template was modified, false if SCHED_MESSAGE did not
     * need to be deleted
     */
    bool clear_log();

    /**
     *  Return ids of NICs with NETWORK_MODE=auto (i.e. need to schedule networks)
     */
    set<int> get_nics_ids()
    {
        return nics_ids_auto;
    }

protected:
    /**
     *  For constructors
     */
    void init_attributes();

    void init_storage_usage();

    /* -------------------.-- SCHEDULER INFORMATION ------------------------- */

    ResourceMatch match_hosts;

    ResourceMatch match_datastores;


    bool only_public_cloud;

    set<int> affined_vms;

    /* ----------------------- VIRTUAL MACHINE ATTRIBUTES ------------------- */
    int   _oid;

    int   _uid;
    int   _gid;

    int   _hid;
    int   _dsid;

    int   resched;
    bool  resume;

    int   state;

    long int    memory;
    float       cpu;
    long long   system_ds_usage;

    map<int,long long> ds_usage;

    bool   public_cloud;

    string rank;
    string requirements;

    string ds_requirements;
    string ds_rank;

    time_t stime;

    set<int> nics_ids_auto;

    map<int, VirtualMachineNicXML *> nics;

    VirtualMachineTemplate * vm_template;   /**< The VM template */
    VirtualMachineTemplate * user_template; /**< The VM user template */
};

#endif /* VM_XML_H_ */

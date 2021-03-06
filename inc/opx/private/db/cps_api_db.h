/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_


#include "cps_api_db_connection.h"
#include "cps_api_operation.h"


#include <algorithm>
#include <functional>
#include <stddef.h>

#define CPS_DB_MAX_ITEMS_PER_SCAN "1000"
#define CPS_DB_MAX_ITEMS_PER_PIPELINE 200

namespace cps_db {

    bool dbkey_from_class_key(std::vector<char> &lst, const cps_api_key_t *key);
    bool dbkey_from_instance_key(std::vector<char> &lst,cps_api_object_t obj, bool escape);
    bool dbkey_instance_or_wildcard(std::vector<char> &lst, cps_api_object_t obj, bool &is_wildcard);
    bool db_key_copy_with_escape(std::vector<char> &lst,const void *data, size_t len);

    static inline cps_api_qualifier_t QUAL(cps_api_key_t *key) { return cps_api_key_get_qual(key); }

    static inline cps_api_qualifier_t UPDATE_QUAL(cps_api_key_t *key, cps_api_qualifier_t qual) {
        cps_api_qualifier_t _cur = cps_api_key_get_qual(key);
        cps_api_key_set(key,0,qual);
        return _cur;
    }
}

namespace cps_db {
    bool delete_object(cps_db::connection &conn,const char *key, size_t key_len);

    bool atomic_count_set(cps_db::connection &conn,const char *key, size_t key_len, int64_t data);
    bool atomic_count_change(cps_db::connection &conn,bool inc, const char *key, size_t key_len,
            int64_t &data);

    bool walk_keys(cps_db::connection &conn, const void *filt, size_t flen,
            const std::function<void(const void *key, size_t klen)> &fun);
}

namespace cps_db {
    bool get_object(cps_db::connection &conn, cps_api_object_t obj);

    bool delete_object(cps_db::connection &conn,cps_api_object_t obj);

    bool get_objects_bulk(cps_db::connection &conn, std::vector<std::vector<char>> &keys,
            cps_api_object_list_t objs);

    bool get_objects(cps_db::connection &conn,std::vector<char> &key,cps_api_object_list_t obj_list) ;
    bool get_objects(cps_db::connection &conn, cps_api_object_t obj,cps_api_object_list_t obj_list);


    bool ping(cps_db::connection &conn);

    bool make_slave(cps_db::connection &conn, std::string slave_ip);
    bool remove_slave(cps_db::connection &conn);

    bool select_db(cps_db::connection &conn,const std::string &id);

    cps_api_return_code_t cps_api_db_init();
}

/*
 * Optimized for pipelining
 * */
namespace cps_db {
    bool set_object_request(cps_db::connection &conn, cps_api_object_t obj,
            bool *check_exists=nullptr, size_t *lifetime=nullptr);
    bool set_object_response(cps_db::connection &conn);

    //Get and pipeline gets
    bool get_object_request(cps_db::connection &conn, const char*key, size_t len);
    bool get_object_response(cps_db::connection &conn, cps_api_object_t obj);

    bool get_object_list(cps_db::connection &conn,cps_api_object_list_t objs);

    bool store_objects(cps_db::connection &conn,cps_api_object_list_t objs);
    bool merge_objects(cps_db::connection &conn, cps_api_object_list_t obj_list);

    bool delete_object_list(cps_db::connection &conn,cps_api_object_list_t objs);
}

/**
 * Events and event subscription
 */
namespace cps_db {
    bool subscribe(cps_db::connection &conn, cps_api_object_t obj);
    bool subscribe(cps_db::connection &conn, std::vector<char> &key);
    bool publish(cps_db::connection &conn, cps_api_object_t obj);
}



/**
 * Wrappers to the optimized functions to handle C++ constructs when available
*/
namespace cps_db {
    static inline bool delete_object(cps_db::connection &conn,const std::vector<char> &key) {
        return cps_db::delete_object(conn,&key[0],key.size());
    }
    static inline bool get_sequence(cps_db::connection &conn, std::vector<char> &key, int64_t &cntr) {
        return cps_db::atomic_count_change(conn,true,&key[0],key.size(),cntr);
    }
    static inline bool get_object(cps_db::connection &conn, const std::vector<char> &key, cps_api_object_t obj) {
        if(get_object_request(conn,&key[0],key.size())) {
            return get_object_response(conn,obj);
        }
        return false;
    }
    static inline bool store_object(cps_db::connection &conn,cps_api_object_t obj) {
        return set_object_request(conn,obj,nullptr,nullptr) && set_object_response(conn);
    }
}


#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_ */

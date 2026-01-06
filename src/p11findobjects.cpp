// -*- mode: c++; c-file-style:"stroustrup"; -*-

//
// Copyright (c) 2025 Mastercard
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <botan/auto_rng.h>
#include "p11findobjects.hpp"


P11FindObjectsBenchmark::P11FindObjectsBenchmark(const std::string &label) :
    P11Benchmark( "Find objects (C_FindObjectsInit-C_FindObjects-C_FindObjectsFinal)", label, ObjectClass::SecretKey ) { }

P11FindObjectsBenchmark::P11FindObjectsBenchmark(const P11FindObjectsBenchmark &other) :
    P11Benchmark(other) { }

inline P11FindObjectsBenchmark *P11FindObjectsBenchmark::clone() const {
    return new P11FindObjectsBenchmark{*this};
}

void P11FindObjectsBenchmark::prepare(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    // The vector size determines the number of temporary keys to create
    size_t num_objects = m_payload.size();
    
    // Clear any previous temporary keys
    m_temp_keys.clear();
    
    // Generate temporary AES keys with unique labels
    
    for (size_t i = 0; i < num_objects; i++) {
        // Generate unique label for each temporary key
        std::stringstream label_stream;
        label_stream << build_threaded_label(threadindex) << "-tmp-" << std::setw(6) << std::setfill('0') << i;
        std::string temp_label = label_stream.str();
        
        // Create attribute template for temporary AES secret key
        AttributeContainer key_template;
        key_template.add_class(ObjectClass::SecretKey);
        key_template.add_string(AttributeType::Label, temp_label);
        key_template.add_numeric(AttributeType::KeyType, static_cast<Ulong>(KeyType::Aes));
        key_template.add_bool(AttributeType::Token, false); // Session object
        key_template.add_bool(AttributeType::Private, true);
        key_template.add_numeric(AttributeType::ValueLen, static_cast<Ulong>(16)); // 128 bits
        
        auto aesmech = CK_MECHANISM{CKM_AES_KEY_GEN, nullptr, 0};
        auto key_handle = CK_OBJECT_HANDLE{};

        // Create the key object
        session.module()->C_GenerateKey(
            session.handle(),
            &aesmech,
            key_template.data(),
            key_template.count(),
            &key_handle
        );
        
        m_temp_keys.push_back(key_handle);
    }
    
    // Generate random indices for each iteration
    // We'll generate 512 random indices and recycle them modulo
    // We go random, in trying to annihilate any caching effects
    size_t num_indices = 512;
    m_random_indices.clear();
    m_random_indices.reserve(num_indices);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, num_objects - 1);
    
    for (size_t i = 0; i < num_indices; i++) {
        m_random_indices.push_back(dis(gen));
    }
    
    // Prepare base label template with placeholder for last 3 digits
    // Format: "<thread_label>-tmp-000000"
    std::stringstream base_stream;
    base_stream << build_threaded_label(threadindex) << "-tmp-000000";
    m_base_label = base_stream.str();
    
    // Prepare search template (will reuse and only modify the label)
    m_search_template.add_class(ObjectClass::SecretKey);
    m_search_template.add_string(AttributeType::Label, m_base_label);
    
    // Reset iteration counter
    m_current_iteration = 0;
}

void P11FindObjectsBenchmark::crashtestdummy(Session &session)
{
    // Get the random index for this iteration (wrap around if needed)
    size_t target_index = m_random_indices[m_current_iteration % m_random_indices.size()];
    m_current_iteration++;
    
    // The following is a bit of a hack to modify 
    // only the last 3 digits of the label within the search template
 
    // Get pointer to the label string data
    char* label_data = static_cast<char*>(m_search_template.attributes()[1].pValue);
    size_t label_len = m_search_template.attributes()[1].ulValueLen;
    
    // Format the last 3 digits: units, tens, hundreds
    label_data[label_len - 1] = '0' + (target_index % 10);
    label_data[label_len - 2] = '0' + ((target_index / 10) % 10);
    label_data[label_len - 3] = '0' + ((target_index / 100) % 10);

    // Now perform the FindObjects operations
    // C_FindObjectsInit
    session.module()->C_FindObjectsInit(
        session.handle(),
        m_search_template.data(),
        static_cast<Ulong>(m_search_template.count())
    );
    
    // C_FindObjects - search for the target object
    ObjectHandle found_object;
    Ulong found_count = 0;
    session.module()->C_FindObjects(
        session.handle(),
        &found_object,
        1,  // max_object_count
        &found_count
    );
    
    // C_FindObjectsFinal
    session.module()->C_FindObjectsFinal(session.handle());
    
    // Verify we found exactly one object
    if (found_count != 1) {
        throw benchmark_result::NotFound();
    }
}

void P11FindObjectsBenchmark::cleanup(Session &session)
{
    // No per-iteration cleanup needed for FindObjects
}

void P11FindObjectsBenchmark::teardown(Session &session, Object &obj, std::optional<size_t> threadindex)
{
    // Destroy all temporary keys created during prepare
    // This is called once after all iterations are complete
    for (auto handle : m_temp_keys) {
        try {
            session.module()->C_DestroyObject(session.handle(), handle);
        } catch (const std::exception& e) {
            std::cerr << "Error destroying temporary object: " << e.what() << std::endl;
        }
    }
    m_temp_keys.clear();
}

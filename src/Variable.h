
#pragma once
#include "Type.h"
#include "Value.h"
#include "util.h"

struct Context;

struct VariableMgr {
    VariableMgr(const std::string name) : _name(name) {}
    Value& get(const std::string& name) {
        if (!_mgr.contains(name)) {
            _mgr[name] = (Value::default_value);
        }
        return _mgr[name];
    }

    bool find(const std::string& name) { return _mgr.contains(name); }

    void set(const std::string& name, const Value& v) { get(name) = v; }

    ReferenceWrapping& get_ref(const std::string name) {
        if (_ref_mgr.contains(name)) {
            return _ref_mgr[name];
        }
        if (!_mgr.contains(name)) {
            _mgr[name] = (Value::default_value);
        }
        _ref_mgr.insert({name, {_mgr.find(name), this}});
        return _ref_mgr[name];
    }

private:
    friend class ReferenceWrapping;
    friend class RefValue;
    std::map<std::string, Value> _mgr;
    std::map<std::string, ReferenceWrapping> _ref_mgr;
    std::string _name;
};

struct StructInfoMgr {


    void put_struct_info(const std::string& name, const std::map<std::string, Value>& map) {
        if (_mgr.contains(name)) {
            auto new_map = std::get<StructValue>(_mgr[name].value()).map();
            for (auto& [k, v] : map) {
                new_map[k] = v;
            }
            _mgr.erase(name);
            put(name, Value::make_Struct(new_map));
        } else {
            put(name, Value::make_Struct(map));
        }
    }

    void extends(const std::string& name, const std::string& base) { _mgr[name] = _mgr[base]; }
    Value get_default_struct_value(const std::string& name) {
        CHECK(_mgr.contains(name), " can not find struct name : " + name);
        return _mgr[name];
    }

    private:
    void put(const std::string& name, Value struct_v) {
        CHECK(struct_v.is_struct(), " must be a struct value");
        _mgr.insert({name, struct_v});
    }

    std::map<std::string, Value> _mgr;
};
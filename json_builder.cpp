#include "json_builder.h"

namespace json
{
    DictKeyContext Builder::Key(std::string &key)
    {
        if (nodes_stack_.empty() || !nodes_stack_.back().IsDict() || last_command == "Key"s || object_completed)
        {
            throw std::logic_error("Invalid call to the \"Key\" method"s);
        }
        keys.emplace_back(std::move(key));
        last_command = "Key"s;
        command_permission_to_insert = true;
        DictKeyContext dict_key_context(*this);
        return dict_key_context;
    }

    Builder Builder::Value(const Node::Value &value)
    {
        if (object_completed || !command_permission_to_insert)
        {
            throw std::logic_error("Invalid call to the \"Value\" method"s);
        }
        if (!nodes_stack_.empty())
        {
            if (nodes_stack_.back().IsDict())
            {
                Dict dict = nodes_stack_.back().AsDict();
                dict[keys.back()] = GetValue(value);
                nodes_stack_.back() = std::move(dict);
                keys.pop_back();
                command_permission_to_insert = false;
            }
            else if (nodes_stack_.back().IsArray())
            {
                Array array = nodes_stack_.back().AsArray();
                array.emplace_back(GetValue(value));
                nodes_stack_.back() = std::move(array);
                command_permission_to_insert = true;
            }
        }
        else
        {
            nodes_stack_.emplace_back(GetValue(value));
            object_completed = true;
        }
        last_command = "Value"s;
        return *this;
    }

    StartDictContext Builder::StartDict()
    {
        if (object_completed || !command_permission_to_insert)
        {
            throw std::logic_error("Invalid call to the \"StartDict\" method"s);
        }
        nodes_stack_.emplace_back(Dict{});
        ++number_unclosed_containers;
        last_command = "StartDict"s;
        command_permission_to_insert = false;
        StartDictContext start_dict_context(*this);
        return start_dict_context;
    }

    StartArrayContext Builder::StartArray()
    {
        if (object_completed || !command_permission_to_insert)
        {
            throw std::logic_error("Invalid call to the \"StartDict\" method"s);
        }
        nodes_stack_.emplace_back(Array{});
        ++number_unclosed_containers;
        last_command = "StartArray"s;
        command_permission_to_insert = true;
        StartArrayContext start_array_context(*this);
        return start_array_context;
    }

    Builder &Builder::EndDict()
    {
        if (!nodes_stack_.back().IsDict() || object_completed)
        {
            throw std::logic_error("Invalid call to the \"EndDict\" method"s);
        }
        EndContainer();
        last_command = "EndDict"s;
        return *this;
    }

    Builder &Builder::EndArray()
    {
        if (!nodes_stack_.back().IsArray() || object_completed)
        {
            throw std::logic_error("Invalid call to the \"EndArray\" method"s);
        }
        EndContainer();
        last_command = "EndArray"s;
        return *this;
    }

    Node &Builder::Build()
    {
        if (!object_completed)
        {
            throw std::logic_error("Object being constructed is not ready"s);
        }
        return nodes_stack_[0];
    }

    void Builder::EndContainer()
    {
        if (nodes_stack_.size() >= 2)
        {
            if (nodes_stack_[nodes_stack_.size() - 2].IsArray())
            {
                Array array = nodes_stack_[nodes_stack_.size() - 2].AsArray();
                array.emplace_back(nodes_stack_.back());
                nodes_stack_.pop_back();
                nodes_stack_.back() = std::move(array);
                command_permission_to_insert = true;
            }
            else
            {
                Dict dict = nodes_stack_[nodes_stack_.size() - 2].AsDict();
                dict[keys.back()] = nodes_stack_.back();
                keys.pop_back();
                nodes_stack_.pop_back();
                nodes_stack_.back() = std::move(dict);
                command_permission_to_insert = false;
            }
        }
        --number_unclosed_containers;
        if (nodes_stack_.size() == 1 && number_unclosed_containers == 0)
        {
            object_completed = true;
        }
    }

    Node Builder::GetValue(const Node::Value &value)
    {
        const auto index = value.index();
        if (index == 0)
        {
            return Node{std::get<0>(value)};
        }
        else if (index == 1)
        {
            return Node{std::get<1>(value)};
        }
        else if (index == 2)
        {
            return Node{std::get<2>(value)};
        }
        else if (index == 3)
        {
            return Node{std::get<3>(value)};
        }
        else if (index == 4)
        {
            return Node{std::get<4>(value)};
        }
        else if (index == 5)
        {
            return Node{std::get<5>(value)};
        }
        else
        {
            return Node{std::get<6>(value)};
        }
    }

    DictKeyContext DictValueContext::Key(std::string key)
    {
        return builder_->Key(key);
    }

    Builder &DictValueContext::EndDict()
    {
        return builder_->EndDict();
    }

    ArrayValueContext ArrayValueContext::Value(const Node::Value &value)
    {
        builder_->Value(value);
        return *this;
    }

    StartDictContext ArrayValueContext::StartDict()
    {
        return builder_->StartDict();
    }

    StartArrayContext ArrayValueContext::StartArray()
    {
        return builder_->StartArray();
    }

    Builder &ArrayValueContext::EndArray()
    {
        return builder_->EndArray();
    }

    DictValueContext DictKeyContext::Value(const Node::Value &value)
    {
        builder_->Value(value);
        DictValueContext dict_value_context(*builder_);
        return dict_value_context;
    }

    StartDictContext DictKeyContext::StartDict()
    {
        return builder_->StartDict();
    }

    StartArrayContext DictKeyContext::StartArray()
    {
        return builder_->StartArray();
    }

    DictKeyContext StartDictContext::Key(std::string key)
    {
        return builder_->Key(key);
    }

    Builder &StartDictContext::EndDict()
    {
        return builder_->EndDict();
    }

    ArrayValueContext StartArrayContext::Value(const Node::Value &value)
    {
        builder_->Value(value);
        ArrayValueContext array_value_context(*builder_);
        return array_value_context;
    }

    StartDictContext StartArrayContext::StartDict()
    {
        return builder_->StartDict();
    }

    StartArrayContext StartArrayContext::StartArray()
    {
        return builder_->StartArray();
    }

    Builder &StartArrayContext::EndArray()
    {
        return builder_->EndArray();
    }

}
#pragma once

#include "json.h"

using namespace std::string_literals;

namespace json
{
    class DictKeyContext;
    class StartDictContext;
    class StartArrayContext;

    class Builder
    {
    public:
        Builder() = default;

        virtual DictKeyContext Key(std::string &key);

        Builder Value(const Node::Value &value);

        StartDictContext StartDict();

        StartArrayContext StartArray();

        Builder &EndDict();

        Builder &EndArray();

        Node &Build();

    private:
        std::vector<Node> nodes_stack_;
        std::vector<std::string> keys;
        int number_unclosed_containers = 0;
        std::string last_command;
        bool object_completed = false;
        bool command_permission_to_insert = true;

        void EndContainer();

        Node GetValue(const Node::Value &value);
    };

    class DictValueContext
    {
    public:
        DictValueContext(Builder &builder)
            : builder_(&builder)
        {
        }

        DictKeyContext Key(std::string key);

        Builder &EndDict();

    private:
        Builder *builder_;
    };

    class ArrayValueContext
    {
    public:
        ArrayValueContext(Builder &builder)
            : builder_(&builder)
        {
        }

        ArrayValueContext Value(const Node::Value &value);

        StartDictContext StartDict();

        StartArrayContext StartArray();

        Builder &EndArray();

    private:
        Builder *builder_;
    };

    class DictKeyContext
    {
    public:
        DictKeyContext(Builder &builder)
            : builder_(&builder)
        {
        }

        DictValueContext Value(const Node::Value &value);

        StartDictContext StartDict();

        StartArrayContext StartArray();

    private:
        Builder *builder_;
    };

    class StartDictContext
    {
    public:
        StartDictContext(Builder &builder)
            : builder_(&builder)
        {
        }

        DictKeyContext Key(std::string key);

        Builder &EndDict();

    private:
        Builder *builder_;
    };

    class StartArrayContext
    {
    public:
        StartArrayContext(Builder &builder)
            : builder_(&builder)
        {
        }

        ArrayValueContext Value(const Node::Value &value);

        StartDictContext StartDict();

        StartArrayContext StartArray();

        Builder &EndArray();

    private:
        Builder *builder_;
    };
}
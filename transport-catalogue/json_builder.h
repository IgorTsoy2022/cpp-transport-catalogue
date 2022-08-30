#pragma once

#include "json.h"

#include <deque>

namespace json {

    class ArrayContext;
    class DictContext;
    class DictKeyContext;
    class ArrayValueContext;
    class DictValueContext;

    class Builder {
    public:
        Builder() = default;

        ArrayContext StartArray();
        Builder& EndArray();

        DictContext StartDict();
        DictKeyContext Key(/*const*/ std::string);
        Builder& EndDict();

        Builder& Value(Node);

        Node& Build();

    private:
        Node root_;
        std::deque<Node*> nodes_ptrs_;

        Node* InsertNode(Node);
    };

    class ArrayContext {
    public:
        ArrayContext(Builder& builder)
            : builder_(builder)
        {}

        ArrayContext StartArray();
        Builder& EndArray();
        DictContext StartDict();
        ArrayValueContext Value(Node);

    private:
        Builder& builder_;
    };

    class DictContext {
    public:
        DictContext(Builder& builder)
            : builder_(builder)
        {}

        DictKeyContext Key(std::string);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

    class DictKeyContext {
    public:
        DictKeyContext(Builder& builder)
            : builder_(builder)
        {}

        ArrayContext StartArray();
        DictContext StartDict();
        DictValueContext Value(Node);

    private:
        Builder& builder_;
    };

    class ArrayValueContext {
    public:
        ArrayValueContext(Builder& builder)
            : builder_(builder)
        {}

        ArrayContext StartArray();
        DictContext StartDict();
        ArrayValueContext Value(Node);
        Builder& EndArray();

    private:
        Builder& builder_;
    };

    class DictValueContext {
    public:
        DictValueContext(Builder& builder)
            : builder_(builder)
        {}

        DictKeyContext Key(std::string);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

} // namespace json
#include "json_builder.h"

namespace json {

    using namespace std::string_literals;

    //-------------------------Builder-------------------------

    ArrayContext Builder::StartArray() {
        nodes_ptrs_.emplace_back(InsertNode(std::move(Array{})));

        ArrayContext context(*this);
        return context;
    }

    Builder& Builder::EndArray() {
        if (nodes_ptrs_.empty()) {
            throw std::logic_error(
                "Attempt to close a none-existent array!"s);
        }

        if (!(nodes_ptrs_.back()->IsArray())) {
            throw std::logic_error(
                "Attempt to close a none-array object!"s);
        }

        nodes_ptrs_.pop_back();

        return *this;
    }

    DictContext Builder::StartDict() {
        nodes_ptrs_.emplace_back(InsertNode(std::move(Dict{})));

        DictContext context(*this);
        return context;
    }

    DictKeyContext Builder::Key(std::string key) {
        if (nodes_ptrs_.empty()) {
            throw std::logic_error(
                "Object map was not initialized!"s);
        }

        Node* node_ptr = nodes_ptrs_.back();

        if (!node_ptr->IsDict()) {
            throw std::logic_error(
                "Object map was not initialized!"s);
        }

        Dict& dict = std::get<Dict>(node_ptr->GetMutableValue());
        dict.emplace(key, key);
        nodes_ptrs_.emplace_back(&dict.at(key));

        DictKeyContext context(*this);
        return context;
    }

    Builder& Builder::EndDict() {
        if (nodes_ptrs_.empty()) {
            throw std::logic_error(
                "Attempt to close a none-existent map!"s);
        }

        if (!(nodes_ptrs_.back()->IsDict())) {
            throw std::logic_error(
                "Attempt to close a none-map object!"s);
        }

        nodes_ptrs_.pop_back();

        return *this;
    }

    Builder& Builder::Value(Node node) {
        InsertNode(std::move(node));
        return *this;
    }

    Node& Builder::Build() {
        if (root_.IsNull()) {
            throw std::logic_error(
                "Object was not comleted!"s);
        }

        if (!nodes_ptrs_.empty()) {
            throw std::logic_error(
                "Object was not comleted!"s);
        }

        return root_;
    }

    Node* Builder::InsertNode(Node node) {
        if (nodes_ptrs_.empty()) {
            if (!root_.IsNull()) {
                throw std::logic_error(
                    "Attempting to write in the wrong place!"s);
            }
            root_ = std::move(node);
            return &root_;
        }

        Node* node_ptr = nodes_ptrs_.back();

        if (node_ptr->IsArray()) {
            Array& array =
                std::get<Array>(node_ptr->GetMutableValue());
            array.push_back(std::move(node));
            return &array.back();
        }

        if (node_ptr->IsString()) {
            const std::string key = node_ptr->AsString();
            nodes_ptrs_.pop_back();
            node_ptr = nodes_ptrs_.back();
            if (node_ptr->IsDict()) {
                Dict& dict =
                    std::get<Dict>(node_ptr->GetMutableValue());
                dict[key] = std::move(node);
                return &dict[key];
            }
        }

        throw std::logic_error(
            "Unexpected type of data!"s);

        return nullptr;
    }

    //----------------------ArrayContext-----------------------

    ArrayValueContext ArrayContext::Value(Node node) {
        ArrayValueContext context(builder_.Value(std::move(node)));
        return context;
    }
    DictContext ArrayContext::StartDict() {
        DictContext context(builder_.StartDict());
        return context;
    }
    ArrayContext ArrayContext::StartArray() {
        ArrayContext context(builder_.StartArray());
        return context;
    }
    Builder& ArrayContext::EndArray() {
        return builder_.EndArray();
    }

    //-----------------------DictContext-----------------------

    DictKeyContext DictContext::Key(/*const*/ std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder& DictContext::EndDict() {
        return builder_.EndDict();
    }

    //----------------------DictKeyContext---------------------

    ArrayContext DictKeyContext::StartArray() {
        ArrayContext context(builder_.StartArray());
        return context;
    }

    DictContext DictKeyContext::StartDict() {
        return builder_.StartDict();
    }

    DictValueContext DictKeyContext::Value(Node node) {
        DictValueContext context(builder_.Value(std::move(node)));
        return context;
    }

    //--------------------ArrayValueContext--------------------

    ArrayContext ArrayValueContext::StartArray() {
        ArrayContext context(builder_.StartArray());
        return context;
    }

    Builder& ArrayValueContext::EndArray() {
        return builder_.EndArray();
    }

    DictContext ArrayValueContext::StartDict() {
        DictContext context(builder_.StartDict());
        return context;
    }

    ArrayValueContext ArrayValueContext::Value(Node node) {
        ArrayValueContext context(builder_.Value(std::move(node)));
        return context;
    }

    //---------------------DictValueContext--------------------

    DictKeyContext DictValueContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder& DictValueContext::EndDict() {
        return builder_.EndDict();
    }

} //namespace json
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

// ---------------------------- JSON ----------------------------

namespace json {

    class Node;
    using Array = std::vector<Node>;
    using Dict = std::map<std::string, Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict,
            bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t);
        Node(Array array);
        Node(Dict map);
        Node(bool value);
        Node(int value);
        Node(double value);
        Node(std::string value);

        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;

        const Value& GetValue() const;

        bool operator==(const Node& other) const {
            return value_ == other.value_;
        }

        bool operator!=(const Node& other) const {
            return !(*this == other);
        }

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const {
            return root_ == other.root_;
        }

        bool operator!=(const Document& other) const {
            return !(*this == other);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
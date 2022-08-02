#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool,
        int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;

        Node(const Value& value);

        bool IsNull() const;

        bool IsBool() const;
        bool AsBool() const;

        bool IsInt() const;
        int AsInt() const;

        bool IsPureDouble() const;
        bool IsDouble() const;
        double AsDouble() const;

        bool IsString() const;
        const std::string& AsString() const;

        bool IsArray() const;
        const Array& AsArray() const;

        bool IsDict() const;
        const Dict& AsDict() const;

        const Value& GetValue() const;

        Value& GetMutableValue();

        bool operator==(const Node& rhs) const;
    };

    inline bool operator!=(const Node& lhs, const Node& rhs);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs,
        const Document& rhs);

    inline bool operator!=(const Document& lhs,
        const Document& rhs);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    void PrintN(const Node& node, std::ostream& output);

}  // namespace json
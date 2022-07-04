#include "json.h"

namespace json {

    using namespace std::literals;

    namespace {

        // ------------------------ Node ------------------------

        Node LoadNode(std::istream& input);

        Node LoadNull(std::istream& input) {
            int charcount = 0;
            std::string result;

            while (input && charcount++ < 3) {
                result += input.get();
            }

            if (result == "ull"sv) {
                return Node();
            }

            throw ParsingError("Wrong null constant"s);
        }

        Node LoadFalse(std::istream& input) {
            int charcount = 0;
            std::string result;

            while (input && charcount++ < 4) {
                result += input.get();
            }

            if (result == "alse"sv) {
                return Node(false);
            }

            throw ParsingError("Wrong boolean constant"s);
        }


        Node LoadTrue(std::istream& input) {
            int charcount = 0;
            std::string result;

            while (input && charcount++ < 3) {
                result += input.get();
            }

            if (result == "rue"sv) {
                return Node(true);
            }

            throw ParsingError("Wrong boolean constant"s);
        }

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError(
                        "Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из
            // input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при
                        // переполнении, код ниже попробует
                        // преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s +
                    parsed_num + " to number"s);
            }

        }

        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили
                    // закрывающую кавычку?
                    throw ParsingError("String parsing error"s);
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа
                        // обратной косой черты
                        throw ParsingError(
                            "String parsing error"s);
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей:
                    //  \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-
                        // последовательность
                        throw ParsingError(
                            "Unrecognized escape sequence \\"s +
                            escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может
                    // прерываться символами \r или \n
                    throw ParsingError(
                        "Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и
                    // помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadArray(std::istream& input) {
            Array result;
            bool check_close_square_bracket = false;

            for (char c; input >> c;) {
                if (c == ']') {
                    check_close_square_bracket = true;
                    break;
                }

                if (c != ',') {
                    input.putback(c);
                }

                result.push_back(LoadNode(input));
            }

            if (!check_close_square_bracket) {
                throw ParsingError("Array parsing error"s);
            }

            return Node(std::move(result));
        }

        Node LoadDict(std::istream& input) {
            Dict result;
            bool check_close_curly_brace = false;

            for (char c; input >> c;) {
                if (c == '}') {
                    check_close_curly_brace = true;
                    break;
                }

                if (c == ',') {
                    input >> c;
                }

                std::string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (!check_close_curly_brace) {
                throw ParsingError("Map parsing error"s);
            }

            return Node(std::move(result));
        }

        Node LoadNode(std::istream& input) {
            char c;
            input >> c;

            if (!input) {
                throw ParsingError("No data!"s);
            }

            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 'f':
                return LoadFalse(input);
            case 't':
                return LoadTrue(input);
            case 'n':
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace


    // ------------------------- Print --------------------------

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным
        // смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& node, std::ostream& out);

    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }

    void PrintValue(const std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }

    void PrintValue(const bool value, std::ostream& out) {
        out << std::boolalpha << value;
    }

    void PrintValue(const int value, std::ostream& out) {
        out << value;
    }

    void PrintValue(const double value, std::ostream& out) {
        out << value;
    }

    void PrintValue(const std::string value, std::ostream& out) {
        std::string result;
        for (const auto& chr : value) {
            switch (chr) {
            case '\\':
                result += "\\\\"s;
                break;
            case '"':
                result += "\\\""s;
                break;
            case '\n':
                result += "\\n"s;
                break;
            case '\r':
                result += "\\r"s;
                break;
            case '\t':
            default:
                result += chr;
            }
        }
        out << "\""s + result + "\""s;
    }

    void PrintValue(const Dict& value,
        std::ostream& out) {
        out << "{"sv;
        bool is_first = true;
        for (const auto& [key, node] : value) {
            if (is_first) {
                is_first = false;
            }
            else {
                out << ", "sv;
            }
            out << '"' + key << "\": "sv;
            PrintNode(node, out);
        }
        out << "}"sv;
    }

    void PrintValue(const Array& value, std::ostream& out) {
        out << "["sv;
        bool is_first = true;
        for (const auto& node : value) {
            if (is_first) {
                is_first = false;
            }
            else {
                out << ", "sv;
            }
            PrintNode(node, out);
        }
        out << "]"sv;
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit([&out](const auto& value) {
            PrintValue(value, out);
            }, node.GetValue());
    }

    // -------------------------- Node --------------------------

    Node::Node(std::nullptr_t)
        : value_(nullptr)
    {}

    Node::Node(Array array)
        : value_(std::move(array))
    {}

    Node::Node(Dict map)
        : value_(std::move(map))
    {}

    Node::Node(bool value)
        : value_(value)
    {}

    Node::Node(int value)
        : value_(value)
    {}

    Node::Node(double value)
        : value_(value)
    {}

    Node::Node(std::string value)
        : value_(std::move(value))
    {}

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return IsInt() || IsPureDouble();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value_);
        }
        throw std::logic_error("Invalid type"s);
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(value_);
        }
        throw std::logic_error("Invalid type"s);
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        }
        throw std::logic_error("Invalid type"s);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        throw std::logic_error("Invalid type"s);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        throw std::logic_error("Invalid type"s);
    }

    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value_);
        }
        throw std::logic_error("Invalid type"s);
    }

    const Node::Value& Node::GetValue() const {
        return value_;
    }

    // ------------------------ Document ------------------------

    Document::Document(Node root)
        : root_(std::move(root))
    {}

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }

    // ------------------------- Print --------------------------

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json
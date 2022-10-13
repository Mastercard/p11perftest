/*
  ConsoleTable borrowed from https://github.com/766F6964/ConsoleTable.git

  Original license here below (MIT License)

  Copyright 2020 766F6964

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "ConsoleTable.h"

ConsoleTable::ConsoleTable(std::initializer_list<std::string> headers) : headers{headers} {
    for (const auto &column : headers) {
	widths.push_back(column.length());
    }
}


void ConsoleTable::setPadding(unsigned int n) {
    padding = n;
}


void ConsoleTable::setStyle(unsigned int n) {
    switch (n) {
	case 0 :
	    style = BasicStyle;
	    break;
	case 1 :
	    style = LineStyle;
	    break;
	case 2 :
	    style = DoubleLineStyle;
	    break;
	case 3 :
	    style = InvisibleStyle;
	    break;
	default :
	    style = BasicStyle;
	    break;
    }
}


bool ConsoleTable::addRow(std::initializer_list<std::string> row) {
    if (row.size() > widths.size()) {
	throw std::invalid_argument{"Appended row size must be same as header size"};
    }

    auto r = std::vector<std::string>{row};
    rows.push_back(r);
    for (unsigned int i = 0; i < r.size(); ++i) {
	widths[i] = std::max(r[i].size(), widths[i]);
    }
    return true;
}


bool ConsoleTable::removeRow(unsigned int index) {
    if (index > rows.size())
	return false;

    rows.erase(rows.begin() + index);
    return true;
}


ConsoleTable &ConsoleTable::operator+=(std::initializer_list<std::string> row) {
    if (row.size() > widths.size()) {
	throw std::invalid_argument{"Appended row size must be same as header size"};
    }

    addRow(row);
    return *this;
}


ConsoleTable &ConsoleTable::operator-=(const uint32_t rowIndex) {
    if (rows.size() < rowIndex)
	throw std::out_of_range{"Row index out of range."};

    removeRow(rowIndex);
    return *this;
}


std::string ConsoleTable::getLine(RowType rowType) const {
    std::stringstream line;
    line << rowType.left;
    for (unsigned int i = 0; i < widths.size(); ++i) {
	for (unsigned int j = 0; j < (widths[i] + padding + padding); ++j) {
	    line << style.horizontal;
	}
	line << (i == widths.size() - 1 ? rowType.right : rowType.intersect);
    }
    return line.str() + "\n";
}


std::string ConsoleTable::getHeaders(Headers headers) const {
    std::stringstream line;
    line << style.vertical;
    for (unsigned int i = 0; i < headers.size(); ++i) {
	std::string text = headers[i];
	line << SPACE_CHARACTER * padding + text + SPACE_CHARACTER * (widths[i] - text.length()) + SPACE_CHARACTER * padding;
	line << style.vertical;
    }
    line << "\n";
    return line.str();
}


std::string ConsoleTable::getRows(Rows rows) const {
    std::stringstream line;
    for (auto &row : rows) {
	line << style.vertical;
	for (unsigned int j = 0; j < row.size(); ++j) {
	    std::string text = row[j];
	    line << SPACE_CHARACTER * padding + text + SPACE_CHARACTER * (widths[j] - text.length()) + SPACE_CHARACTER * padding;
	    line << style.vertical;
	}
	line << "\n";
    }

    return line.str();
}


std::ostream &operator<<(std::ostream &out, const ConsoleTable &consoleTable) {
    out << consoleTable.getLine(consoleTable.style.top);
    out << consoleTable.getHeaders(consoleTable.headers);
    out << consoleTable.getLine(consoleTable.style.middle);
    out << consoleTable.getRows(consoleTable.rows);
    out << consoleTable.getLine(consoleTable.style.bottom);
    return out;
}

bool ConsoleTable::sort(bool ascending) {
    if (ascending)
	std::sort(rows.begin(), rows.end(), std::less<std::vector<std::string>>());
    else
	std::sort(rows.begin(), rows.end(), std::greater<std::vector<std::string>>());
    return true;
}

void ConsoleTable::updateRow(unsigned int row, unsigned int header, std::string data) {
    if (row > rows.size() - 1)
	throw std::out_of_range{"Row index out of range."};
    if (header > headers.size() - 1)
	throw std::out_of_range{"Header index out of range."};

    rows[row][header] = data;
}

void ConsoleTable::updateHeader(unsigned int header, std::string text) {
    if (header > headers.size())
	throw std::out_of_range{"Header index out of range."};

    headers[header] = text;
}


std::string operator*(const std::string &other, int repeats) {
    std::string ret;
    ret.reserve(other.size() * repeats);
    for (; repeats; --repeats)
	ret.append(other);
    return ret;
}

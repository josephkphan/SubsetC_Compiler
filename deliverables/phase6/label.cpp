# include <sstream>
# include <iostream>
# include "label.h"

unsigned Label::_counter = 0;

Label::Label() {
    _number = _counter++;
}

unsigned Label::number() const {
    return _number;
}

// std::ostream &operator<< (std::ostream &os, const Label &lbl) {
//     return os << ".L" << lbl.number();
// }
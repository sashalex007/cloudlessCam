#ifndef RESET_H
#define RESET_H

void save_reset(std::vector<String>& base64_vector, int reset_count);
void open_reset(std::vector<String>& base64_vector);
int get_reset_count();
void erase_reset();

#endif
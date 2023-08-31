#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if (data.empty()) {
    if (is_last_substring) {
      output.close();
    }
    return;
  }

  if (output.available_capacity() == 0) {
    return;
  }

  auto last_index = first_index + data.size();

  auto first_unaccepted_index = first_unassembled_index_ + output.available_capacity();

  // 已经写入过
  if (last_index <= first_unassembled_index_ || first_index >= first_unaccepted_index) {
    return;
  }

  if (last_index >= first_unaccepted_index) {
    data = data.substr(0, first_unaccepted_index - first_index);
    if (data.empty()) {
      return;
    }
  }

  is_last_substring_ |= is_last_substring;

  // earlier bytes remain unknown, insert into buffer
  if (first_index > first_unassembled_index_) {
    auto begin_index = first_index;
    auto end_index = first_index + data.size();
    if (bytes_pending_list.empty()) {
      bytes_pending_list.emplace_back(begin_index, data);
      buffer_size_ += data.size();
      return;
    }
    // c 2 bcd 1
    for (auto it = bytes_pending_list.begin(); it != bytes_pending_list.end() && begin_index < end_index;) {
      if (it->first <= begin_index) {
        begin_index = max(begin_index, it->first + it->second.size());
        ++it;
        continue;
      }
      if (begin_index == first_index && end_index <= it->first) {
        bytes_pending_list.emplace(it, begin_index, data);
        buffer_size_ += data.size();
        return;
      }
      auto right_index = min(end_index, it->first);
      auto len = right_index - begin_index;
      bytes_pending_list.emplace(it, begin_index, data.substr(begin_index - first_index, len));
      buffer_size_ += len;
      begin_index = right_index;
    }

    if (begin_index < end_index) {
      bytes_pending_list.emplace_back(begin_index, data.substr(begin_index - first_index));
      buffer_size_ += end_index - begin_index;
    }

    return;
  }

  if (first_index < first_unassembled_index_) {
    data = data.substr(first_unassembled_index_ - first_index);
    first_index = first_unassembled_index_;
  }

  if (first_index == first_unassembled_index_) {
    // 不是链表有什么加什么，而是数据包中没有什么加什么，数据包校验正确后就是对的
    // 把连续的字符串连接一起并清除
    auto expect_index = first_index + data.size();
    for (auto it = bytes_pending_list.begin(); it != bytes_pending_list.end();) {
      if (expect_index < it->first) {
        break;
      }
      if (expect_index > it->first) {
        if (it->first + it->second.size() > expect_index) {
          auto tmp = it->second.substr(expect_index - it->first);
          data += tmp;
          expect_index += tmp.size();
        }
        buffer_size_ -= it->second.size();
        it = bytes_pending_list.erase(it);
        continue;
      }
      data += it->second;
      expect_index += it->second.size();
      buffer_size_ -= it->second.size();
      it = bytes_pending_list.erase(it);
    }
    first_unassembled_index_ += data.size();
    output.push( move(data) );
    if (is_last_substring_ && bytes_pending_list.empty()) {
      output.close();
    }
    return;
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return buffer_size_;
}

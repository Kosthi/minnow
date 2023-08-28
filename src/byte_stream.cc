#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), remaining_capacity_( capacity ) {}

void Writer::push( string data )
{
  if (remaining_capacity_ == 0 || data.empty()) {
    return;
  }
  size_t const len = std::min(data.size(), remaining_capacity_);
  if (len < data.size()) {
    data = data.substr(0, len);
  }
  buffer_.emplace_back(std::move(data));
  buffer_view_.emplace_back(buffer_.back().c_str(), len);
  bytes_pushed_ += len;
  bytes_buffered_ += len;
  remaining_capacity_ -= len;
}

void Writer::close()
{
  is_closed_ = true;
}

void Writer::set_error()
{
  has_error_ = true;
}

bool Writer::is_closed() const
{
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  return remaining_capacity_;
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  if (buffer_view_.empty()) {
    return {};
  }
  return buffer_view_.front();
}

bool Reader::is_finished() const
{
  return is_closed_ && bytes_buffered_ == 0;
}

bool Reader::has_error() const
{
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  len = std::min(len, bytes_buffered_);
  while ( len > 0) {
    size_t sz = buffer_view_.front().size();
    if ( len >= sz ) {
      buffer_view_.pop_front();
    } else {
      buffer_view_.front().remove_prefix( len );
      sz = len;
    }
    len -= sz;
    bytes_popped_ += sz;
    bytes_buffered_ -= sz;
    remaining_capacity_ += sz;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return bytes_buffered_;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

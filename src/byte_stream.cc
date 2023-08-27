#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), remaining_capacity_( capacity ) {}

void Writer::push( string data )
{
  if (remaining_capacity_ == 0) {
    return;
  }
  size_t const len = std::min(data.size(), remaining_capacity_);
  if (len == remaining_capacity_) {
    data = data.substr(0, len);
  }
  buffer_.emplace(data);
  buffer_view_.emplace(buffer_.back().c_str(), len);
  nwrite_ += len;
  remaining_capacity_ -= len;
}

void Writer::close()
{
  is_closed_ = true;
}

void Writer::set_error()
{
  is_errored_ = true;
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
  return nwrite_;
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
  return is_closed_;
}

bool Reader::has_error() const
{
  return is_errored_;
}

void Reader::pop( uint64_t len )
{
  if (len < buffer_view_.front().size()) {
    buffer_view_.front().remove_prefix( len );
    return;
  }
  len -= buffer_view_.front().size();
  buffer_view_.pop();
  buffer_view_.front().remove_prefix( len );
  nread_ += len;
  nwrite_ -= len;
  remaining_capacity_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  return nwrite_;
}

uint64_t Reader::bytes_popped() const
{
  return nread_;
}

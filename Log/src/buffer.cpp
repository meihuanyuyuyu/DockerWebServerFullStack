#include"buffer.hpp"


LogSys::Buffer::Buffer():_buffer(DEFAULT_BUFFER_SIZE),_reader_index(0),_writer_index(0){
}

void LogSys::Buffer::Push(const char *data, size_t len){
    // 剩余空间不足 - 扩容
    ensureEnoughSize(len);
    // 1. 将数据拷贝到缓冲区
    std::copy(data, data + len, &_buffer[_writer_index]);

    // 2.将写入位置后移
    MoveWriter(len);
}

const char *LogSys::Buffer::Begin(){
    return &_buffer[_reader_index];
}

size_t LogSys::Buffer::ReadAbleSize() const{
    // 当前实现的缓冲区设计思想是双缓冲区，处理完就交换，不存在循环
    return (_writer_index - _reader_index);
}

size_t LogSys::Buffer::WriteAbleSize() const{
    // 对于扩容，不存在可写空间大小，因为总是可写
    // 这个接口仅仅针对固定大小缓冲区
    return (_buffer.size() - _writer_index);
}

void LogSys::Buffer::ensureEnoughSize(size_t len){
    if(len<=WriteAbleSize()){
        return;
    }
    size_t new_size = 0;
    if(_buffer.size()<THRESHOLD_BUFFER_SIZE){
        new_size = _buffer.size()*2+len;
    }else{
        new_size = _buffer.size()+INCREMENT_BUFFER_SIZE+len;
    }
    std::vector<char> new_buffer(new_size);
    std::copy(Begin(),Begin()+ReadAbleSize(),new_buffer.begin());
    _buffer.swap(new_buffer);
}

void LogSys::Buffer::MoveWriter(size_t len){
    assert((len+_writer_index)<=_buffer.size());
    _writer_index+=len;
}

void LogSys::Buffer::MoveReader(size_t len){
    assert(len<=ReadAbleSize());
    _reader_index+=len;
}

void LogSys::Buffer::Swap(Buffer &buffer){
    _buffer.swap(buffer._buffer);
    std::swap(_reader_index,buffer._reader_index);
    std::swap(_writer_index,buffer._writer_index);
}

bool LogSys::Buffer::Empty() const{
    return (_reader_index==_writer_index);
}

void LogSys::Buffer::Reset(){
    _reader_index = 0;
    _writer_index = 0;
}




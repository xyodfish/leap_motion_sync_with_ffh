#ifndef __AR_CIRCULAR_BUFFER_H__
#define __AR_CIRCULAR_BUFFER_H__

#include <cassert>
#include <stdexcept>
#include <vector>

namespace ar::Types {
    template <typename T>
    class CircularBuffer {
       public:
        CircularBuffer(size_t size) : m_size(size), m_tail(0) { m_buffer.resize(size); }
        CircularBuffer(const CircularBuffer<T>& other)               = default;
        CircularBuffer<T>& operator=(const CircularBuffer<T>& other) = default;

        CircularBuffer()  = default;
        ~CircularBuffer() = default;

        void setSize(size_t size) {

            m_buffer.clear();
            m_buffer.resize(size);
            m_size = size;
        }

        void add(const T& value) {
            m_buffer[m_tail] = value;
            updateHeadTail();
        }

        /**
         * @brief Get the latest object id
         * 
         * @return size_t 
         */
        size_t getTail() const { return m_tail; }

        /**
         * @brief Get the oldest object id
         * 
         * @return size_t 
         */
        size_t getHead() const { return m_head; }

        /**
         * @brief Get the Size object
         * 
         * @return size_t 
         */
        size_t getSize() const { return m_size; }

        /**
         * @brief Get the Buffer object
         * 
         * @return std::vector<T> 
         */
        std::vector<T> getBuffer() const noexcept { return m_buffer; }

        /**
         * @brief 索引访问
         * 
         * @param id 
         * @return T& 
         */
        T& operator[](size_t id) {
            assert(id < size());
            return m_buffer[id];
        }

        /**
         * @brief get the last value in circular buffer
         * 
         * @return T& 
         */
        T& back() { return m_buffer[m_tail]; }

        /**
         * @brief get the first value in circular buffer
         * 
         * @return T& 
         */
        T& front() { return m_buffer[m_head]; }

        /**
         * @brief Pop item from front.
         * If there are no elements in the container, the behavior is undefined.
         */
        void popFront() {

            if (!empty()) {
                m_head = (m_head + 1) % m_size;
            }
        }

        /**
         * @brief 
         * 
         * @return true 
         * @return false 
         */
        bool empty() const { return m_tail == m_head; }

        /**
         * @brief 
         * 
         * @return true 
         * @return false 
         */
        bool full() const {

            // head is ahead of the tail by 1
            return (m_size > 0) ? ((m_tail + 1) % m_size == m_head) : false;
        }

        // Return number of elements actually stored
        size_t size() const { return (m_tail >= m_head) ? (m_tail - m_head) : (m_size - (m_head - m_tail)); }

       private:
        /**
        * @brief update buffer head and tail id
        * 
        */
        void updateHeadTail() {
            m_tail = (m_tail + 1) % m_size;
            m_head = (m_tail + m_size - 1) % m_size;
        }

        std::vector<T> m_buffer;
        size_t m_size{0}, m_tail{0}, m_head{0};
    };
}  // namespace ar::Types

#endif
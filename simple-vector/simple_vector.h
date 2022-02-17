#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) :capacity_(capacity_to_reserve) {
    }
    size_t capacity_{};
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) :SimpleVector(size, Type{}) {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type> simpleVector(size);
        simpleVector_.swap(simpleVector);
        std::fill(simpleVector_.Get(), simpleVector_.Get() + size, value);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        ArrayPtr<Type> tmp1(init.size());
        size_ = init.size();
        std::copy(init.begin(), init.end(), tmp1.Get());
        simpleVector_.swap(tmp1);
        capacity_ = size_;
    }

    SimpleVector(ReserveProxyObj t) {
        Reserve(t.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_array(new_capacity);
            std::copy(simpleVector_.Get(), simpleVector_.Get() + size_, new_array.Get());
            simpleVector_.swap(new_array);
            capacity_ = new_capacity;
        }
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return  size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return simpleVector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index. Проверить, что индекс не превышает размерности вектора - по ассерт
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return  simpleVector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_)
        {
            throw std::out_of_range("out_of_range");
        }
        return  simpleVector_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_)
        {
            throw std::out_of_range("out_of_range");
        }
        return  simpleVector_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
   // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= capacity_) {
            std::fill(simpleVector_.Get() + size_, simpleVector_.Get() + capacity_, Type());
            size_ = new_size;
        }
        if (new_size > capacity_) {
            size_t old_size = size_;
            if (new_size > 2 * capacity_) {
                Relocation(new_size);
            }
            if (new_size <= 2 * capacity_) {
                Relocation(2 * capacity_);
            }
            std::fill(simpleVector_.Get() + old_size, simpleVector_.Get() + capacity_, Type());
        }
        size_ = new_size;
    }

    SimpleVector(SimpleVector&& other) {
        simpleVector_.swap(other.simpleVector_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        simpleVector_.swap(rhs.simpleVector_);
        size_ = std::exchange(rhs.size_, 0);
        capacity_ = std::exchange(rhs.capacity_, 0);
        return *this;
    }

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> tmp1(other.GetSize());
        size_ = other.GetSize();
        std::copy(other.begin(), other.end(), tmp1.Get());
        simpleVector_.swap(tmp1);
        capacity_ = size_;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            // Реализация операции присваивания с помощью идиомы Copy-and-swap
            // Если исключение будет выброшено, то на текущий объект оно не повлияет
            auto rhs_copy(rhs);
            // rhs_copy содержит копию правого аргумента
            // Обмениваемся с ним данными
           // this->swap(rhs_copy);
            swap(rhs_copy);
            // теперь текущий объект содержит копию правого аргумента,
            // а rhs_copy - прежнее состояние текущего объекта, которое при выходе
            // из блока будет разрушено
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            simpleVector_[size_] = item;
            ++size_;
        }
        else {
            size_t capacity{};
            if (capacity_ == 0) {
                capacity = 1;
            }
            else {
                capacity = 2 * capacity_;
            }
            ArrayPtr<Type> new_array(capacity);
            std::copy(simpleVector_.Get(), simpleVector_.Get() + size_, new_array.Get());
            simpleVector_.swap(new_array);
            simpleVector_[size_] = item;
            ++size_;
            capacity_ = capacity;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            simpleVector_[size_] = std::move(item);
            ++size_;
        }
        else {
            size_t capacity{};
            if (capacity_ == 0) {
                capacity = 1;
            }
            else {
                capacity = 2 * capacity_;
            }
            ArrayPtr<Type> new_array(capacity);
            std::move(std::make_move_iterator(simpleVector_.Get()), std::make_move_iterator(simpleVector_.Get() + size_), new_array.Get());
            simpleVector_.swap(new_array);
            simpleVector_[size_] = std::move(item);
            ++size_;
            capacity_ = capacity;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        size_t capacity{};
        if (capacity_ == 0) {
            capacity_ = 1;
            ++size_;
            ArrayPtr<Type> simpleVector(1);
            simpleVector_.swap(simpleVector);
            simpleVector_[0] = value;
            return simpleVector_.Get();
        }
        else if (size_ == capacity_) {
            capacity = 2 * capacity_;
            ArrayPtr<Type> new_array(capacity);
            std::copy(simpleVector_.Get(), pos, new_array.Get());
            Iterator new_pos = new_array.Get();
            Iterator begin_old = simpleVector_.Get();
            while (begin_old != pos) {
                ++begin_old;
                ++new_pos;
            }
            *new_pos = value;
            ++new_pos;
            std::copy(pos, simpleVector_.Get() + size_, new_pos);
            simpleVector_.swap(new_array);
            ++size_;
            capacity_ = capacity;
            return --new_pos;
        }
        else {
            Iterator new_end = simpleVector_.Get() + size_;
            ++new_end;
            std::copy_backward(pos, simpleVector_.Get() + size_, new_end);
            *pos = value;
            ++size_;
            return pos;
        }
        return pos;
    }

    Iterator Insert(Iterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t capacity{};
        if (capacity_ == 0) {
            capacity_ = 1;
            ++size_;
            ArrayPtr<Type> simpleVector(1);
            simpleVector_.swap(simpleVector);
            simpleVector_[0] = std::move(value);
            return simpleVector_.Get();
        }
        else if (size_ == capacity_) {
            capacity = 2 * capacity_;
            ArrayPtr<Type> new_array(capacity);
            std::move(std::make_move_iterator(simpleVector_.Get()), std::make_move_iterator(pos), new_array.Get());
            Iterator new_pos = std::move(new_array.Get());
            Iterator begin_old = std::move(simpleVector_.Get());
            while (begin_old != pos) {
                ++begin_old;
                ++new_pos;
            }
            *new_pos = std::move(value);
            ++new_pos;
            std::move(std::make_move_iterator(pos), std::make_move_iterator(simpleVector_.Get() + size_), new_pos);
            simpleVector_.swap(new_array);
            ++size_;
            capacity_ = std::move(capacity);
            return --new_pos;
        }
        else {
            Iterator new_end = std::move(simpleVector_.Get() + size_);
            ++new_end;
            std::move_backward(std::make_move_iterator(pos), std::make_move_iterator(simpleVector_.Get() + size_), new_end);
            *pos = std::move(value);
            ++size_;
            return pos;
        }
        return pos;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos <= cend());
        Iterator it_next = const_cast<Iterator>(pos);
        ++it_next;
        std::move(it_next, simpleVector_.Get() + size_, const_cast<Iterator>(pos));
        --size_;
        return --it_next;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        other.simpleVector_.swap(simpleVector_);
        std::swap(other.size_, size_);
        std::swap(other.capacity_, capacity_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return  simpleVector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return  simpleVector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return  simpleVector_.Get();

    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return simpleVector_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return simpleVector_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return simpleVector_.Get() + size_;
    }

private:
    ArrayPtr<Type>simpleVector_{};
    size_t size_{};
    size_t capacity_{};

    void Relocation(size_t new_size) {
        ArrayPtr<Type> new_array(new_size);
        std::copy(simpleVector_.Get(), simpleVector_.Get() + size_, new_array.Get());
        simpleVector_.swap(new_array);
        capacity_ = new_size;
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
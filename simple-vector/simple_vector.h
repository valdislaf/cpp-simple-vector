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
        Array_Ptr<Type>simpleVector(size);
        simpleVector_.swap(simpleVector);
        Iterator it_end = It_move(simpleVector_.Get(), size);
        std::fill(simpleVector_.Get(), it_end, value);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        Array_Ptr<Type> tmp1(init.size());
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
            Array_Ptr<Type> new_array(new_capacity);
            Iterator it_end = It_move(simpleVector_.Get(), size_);
            std::move(std::make_move_iterator(const_cast<Iterator>(simpleVector_.Get())), std::make_move_iterator(const_cast<Iterator>(it_end)), new_array.Get());
            simpleVector_.swap(new_array);
            capacity_ = std::move(new_capacity);
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
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            Iterator it_begin = It_move(simpleVector_.Get(), size_);
            Iterator it_end = It_move(simpleVector_.Get(), capacity_);
            std::fill(it_begin, it_end, Type());
            size_ = new_size;
        }

        if (new_size > capacity_) {
            Iterator it_end = It_move(simpleVector_.Get(), size_);
            size_t sz_pred = size_;
            if (new_size > 2 * capacity_) {
                size_ = new_size;
                new_capacity(new_size);
            }
            if (new_size <= 2 * capacity_) {
                size_ = new_size;
                new_capacity(2 * capacity_);
            }
            Iterator it_begin = It_move(simpleVector_.Get(), sz_pred);
            Iterator it_end2 = It_move(simpleVector_.Get(), capacity_);
            std::fill(it_begin, it_end2, Type());
        }
    }

    Iterator It_move(Iterator it, size_t m) {
        return Iterator{ it + m };
    }

    void new_capacity(size_t new_size) {
        Array_Ptr<Type> new_array(new_size);
        Iterator it_end = It_move(simpleVector_.Get(), size_);
        std::move(std::make_move_iterator(const_cast<Iterator>(simpleVector_.Get())), std::make_move_iterator(const_cast<Iterator>(it_end)), new_array.Get());
        simpleVector_.swap(new_array);
        capacity_ = new_size;
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
        Array_Ptr<Type> tmp1(other.GetSize());
        size_ = std::move(other.GetSize());
        std::copy(other.begin(), other.end(), tmp1.Get());
        simpleVector_.swap(tmp1);
        capacity_ = size_;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {

            // Реализация операции присваивания с помощью идиомы Copy-and-swap
            // Если исключение будет выброшено, то на текущий объект оно не повлияет
            auto rhs_copy(std::move(rhs));
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
        if ((size_ + 1) <= capacity_) {
            simpleVector_[size_] = std::move(const_cast<Type&>(item));
            ++size_;
        }
        else {
            Iterator it_end = It_move(simpleVector_.Get(), size_);
            size_t capacity{};
            if (capacity_ == 0) {
                capacity = 1;
            }
            else {
                capacity = std::move(2 * capacity_);
            }
            Array_Ptr<Type> new_array(capacity);
            std::move(std::make_move_iterator(const_cast<Iterator>(simpleVector_.Get())), std::make_move_iterator(const_cast<Iterator>(it_end)), new_array.Get());
            simpleVector_.swap(new_array);
            simpleVector_[size_] = std::move(const_cast<Type&>(item));
            ++size_;
            capacity_ = std::move(capacity);
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= simpleVector_.Get());
        Iterator it_end = It_move(simpleVector_.Get(), size_);
        assert(pos <= it_end);

        size_t capacity{};
        if (capacity_ == 0) {
            capacity_ = std::move(1);
            size_ = std::move(size_ + 1);
            Array_Ptr<Type>simpleVector(1);
            simpleVector_.swap(simpleVector);
            simpleVector_[0] = std::move(const_cast<Type&>(value));
            return Iterator(simpleVector_.Get());
        }
        else if (size_ == capacity_) {
            capacity = std::move(2 * capacity_);
            Array_Ptr<Type> new_array(capacity);
            Iterator it_end = It_move(simpleVector_.Get(), size_);
            Iterator pred_pos = std::move(const_cast<Iterator>(pos));

            std::move(std::make_move_iterator(const_cast<Iterator>(simpleVector_.Get())), std::make_move_iterator(const_cast<Iterator>(pred_pos)), new_array.Get());

            Iterator New_pos = std::move(new_array.Get());
            Iterator begin_old = std::move(simpleVector_.Get());
            while (begin_old != pos) {
                ++begin_old;
                ++New_pos;
            }
            *New_pos = std::move(const_cast<Type&>(value));
            ++New_pos;
            std::move(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(const_cast<Iterator>(it_end)), New_pos);
            simpleVector_.swap(new_array);


            size_ = std::move(size_ + 1);
            capacity_ = std::move(capacity);
            return Iterator(--New_pos);
        }
        else {
            Iterator it_end = It_move(simpleVector_.Get(), size_);

            Iterator new_end = std::move(const_cast<Iterator>(it_end));
            ++new_end;

            std::move_backward(std::make_move_iterator(const_cast<Iterator>(pos)), std::make_move_iterator(const_cast<Iterator>(it_end)), new_end);

            *const_cast<Iterator>(pos) = std::move(const_cast<Type&>(value));
            size_ = std::move(size_ + 1);
            return Iterator(pos);
        }
        return Iterator(pos);
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= simpleVector_.Get());
        Iterator it_end = It_move(simpleVector_.Get(), size_);
        assert(pos <= it_end);
        ConstIterator it_next = pos;
        ++it_next;
        std::move(std::make_move_iterator(const_cast<Iterator>(it_next)), std::make_move_iterator(const_cast<Iterator>(it_end)), const_cast<Iterator>(pos));
        --size_;
        return Iterator(--it_next); //0x000001e455b05db8 3
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
        return Iterator{ simpleVector_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator{ simpleVector_.Get() + size_ };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator{ simpleVector_.Get() };

    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator{ simpleVector_.Get() + size_ };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{ simpleVector_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{ simpleVector_.Get() + size_ };
    }

private:
    Array_Ptr<Type>simpleVector_{};
    size_t size_{};
    size_t capacity_{};

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


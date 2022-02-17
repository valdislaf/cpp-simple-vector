#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>


template <typename Type>
class Array_Ptr {
public:
    // Инициализирует Array_Ptr нулевым указателем
    Array_Ptr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit Array_Ptr(size_t size) {
        if (size == 0) {
            raw_ptr_ = std::move(nullptr);
        }
        else {
            raw_ptr_ = std::move(new Type[size]);
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit Array_Ptr(Type* raw_ptr) noexcept {
        raw_ptr_ = std::move(raw_ptr);
    }

    // Запрещаем копирование
    Array_Ptr(const Array_Ptr&) = delete;

    ~Array_Ptr() {
        delete[]  raw_ptr_;
    }

    // Запрещаем присваивание
    Array_Ptr& operator=(const Array_Ptr&) = delete;

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* ptr = std::move(raw_ptr_);
        raw_ptr_ = std::move(nullptr);
        return ptr;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {

        return *std::move(raw_ptr_ + index);
    }

    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return *std::move(raw_ptr_ + index);
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const {

        return raw_ptr_ != nullptr;
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {

        return  std::move(raw_ptr_);
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(Array_Ptr& other) noexcept {
        std::swap(other.raw_ptr_, raw_ptr_);
    }

private:
    Type* raw_ptr_ = std::move(nullptr);
};
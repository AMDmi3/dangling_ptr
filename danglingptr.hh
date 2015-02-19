/*
 * Copyright (c) 2015 Dmitry Marakasov <amdmi3@amdmi3.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef DANGLINGPTR_HH
#define DANGLINGPTR_HH

#include <memory>
#include <cassert>
#include <exception>

#if defined(DANGLINGPTR_USE_LIST) && !defined(DANGLINGPTR_USE_SET)
#include <algorithm>
#include <list>
#else
#include <set>
#endif

namespace dangling {

class bad_access : public std::exception {
public:
	bad_access() {
	}

	virtual ~bad_access() noexcept {
	}

	virtual const char* what() const noexcept {
		return "bad dangling::ptr access";
	}
};

template <class T>
class target {
private:
#if defined(DANGLINGPTR_USE_LIST)
	std::list<T**> dangling_ptrs_;
#else
	std::set<T**> dangling_ptrs_;
#endif

public:
	target() {
	}

	virtual ~target() {
		for (auto& ptr : dangling_ptrs_)
			*ptr = nullptr;
	}

	void register_ptr(T** ptr) {
#if defined(DANGLINGPTR_USE_LIST)
		assert(std::find(dangling_ptrs_.begin(), dangling_ptrs_.end(), ptr) == dangling_ptrs_.end());
		dangling_ptrs_.push_back(ptr);
#else
		assert(dangling_ptrs_.find(ptr) == dangling_ptrs_.end());
		dangling_ptrs_.insert(ptr);
#endif
	}

	void unregister_ptr(T** ptr) {
#if defined(DANGLINGPTR_USE_LIST)
		auto iter = std::find(dangling_ptrs_.begin(), dangling_ptrs_.end(), ptr);
		assert(iter != dangling_ptrs_.end());
		dangling_ptrs_.erase(iter);
#else
		auto iter = dangling_ptrs_.find(ptr);
		assert(iter != dangling_ptrs_.end());
		dangling_ptrs_.erase(iter);
#endif
	}
};

template <class T>
class ptr {
private:
	std::unique_ptr<T*> target_;

public:
	// ctor/dtor
	constexpr ptr() {
	}

	explicit ptr(T* target) : target_(target ? new T*(nullptr) : nullptr) {
		if (target != nullptr) {
			target->register_ptr(target_.get());
			*target_ = target;
		}
	}

	~ptr() {
		if (target_ && *target_)
			(*target_)->unregister_ptr(target_.get());
	}

	// move ctor/assignment
	ptr(ptr<T>&&) noexcept = default;

	ptr<T>& operator=(ptr<T>&& other) noexcept {
		if (this == &other)
			return *this;
		if (target_ && *target_)
			(*target_)->unregister_ptr(target_.get());
		target_ = std::move(other.target_);
		return *this;
	}

	// copy ctor/assignment
	ptr(const ptr<T>& other) : target_(other.get() ? new T*(other.get()) : nullptr) {
		if (target_)
			(*target_)->register_ptr(target_.get());
	}

	ptr<T>& operator=(const ptr<T>& other) {
		if (this == &other)
			return *this;
		reset(other.target_ ? *other.target_ : nullptr);
		return *this;
	}

	// setters
	void reset(T* target = nullptr) {
		if (get() == target)
			return;
		if (target) {
			if (!target_)
				target_.reset(new T*(nullptr));
			target->register_ptr(target_.get());
		}
		if (target_ && *target_)
			(*target_)->unregister_ptr(target_.get());
		*target_ = target;
	}

	// dereferencing
	T& operator*() const {
		if (!target_ || *target_ == nullptr)
			throw bad_access();
		return **target_;
	}

	T* operator->() const {
		if (!target_ || *target_ == nullptr)
			throw bad_access();
		return *target_;
	}

	T* get() const noexcept {
		return target_ ? *target_ : nullptr;
	}

	// operators
	explicit operator bool() const noexcept {
		return target_ && *target_ != nullptr;
	}

	bool operator==(const ptr<T>& other) const noexcept {
		return get() == other.get();
	}

	bool operator!=(const ptr<T>& other) const noexcept {
		return get() != other.get();
	}

	bool operator<(const ptr<T>& other) const noexcept {
		return get() < other.get();
	}

	bool operator<=(const ptr<T>& other) const noexcept {
		return get() <= other.get();
	}

	bool operator>(const ptr<T>& other) const noexcept {
		return get() > other.get();
	}

	bool operator>=(const ptr<T>& other) const noexcept {
		return get() >= other.get();
	}
};

}

#endif

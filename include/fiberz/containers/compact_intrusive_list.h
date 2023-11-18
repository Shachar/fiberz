#pragma once

#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <assert.h>

namespace Fiberz::Containers {

template <typename T, size_t NodeOffset>
class CompactIntrusiveList;

class CompactIntrusiveList_Node {
    template<typename T, size_t NodeOffset>
    friend class CompactIntrusiveList;

private:
    CompactIntrusiveList_Node *prev = nullptr, *next = nullptr;

public:
    explicit CompactIntrusiveList_Node() = default;

    // No copying of CompactIntrusiveList_Node
    CompactIntrusiveList_Node( const CompactIntrusiveList_Node &that ) = delete;
    CompactIntrusiveList_Node &operator=( const CompactIntrusiveList_Node &that ) = delete;
    // Nor moving
    CompactIntrusiveList_Node( const CompactIntrusiveList_Node &&that ) = delete;
    CompactIntrusiveList_Node &operator=( const CompactIntrusiveList_Node &&that ) = delete;

    ~CompactIntrusiveList_Node() {
        assert( next==nullptr );
        assert( prev==nullptr );
    }
};


// Cycle doubly linked intrusive list, allowing the list object itself to be a single pointer.
template <typename T, size_t NodeOffset>
class CompactIntrusiveList {
public:
private:
    CompactIntrusiveList_Node *head_ = nullptr;

public:
    using NodePtr = boost::intrusive_ptr<T>;
    using NodeCPtr = boost::intrusive_ptr<const T>;

    explicit CompactIntrusiveList() = default;

    // Can't copy
    CompactIntrusiveList( const CompactIntrusiveList &that ) = delete;
    CompactIntrusiveList &operator=( const CompactIntrusiveList &that ) = delete;

    // Can move
    CompactIntrusiveList( CompactIntrusiveList &&that ) : head_(that.head_) {
        that.head_ = nullptr;
    }
    CompactIntrusiveList &operator=( CompactIntrusiveList that ) {
        swap(that);
    }

    ~CompactIntrusiveList() {
        clear();
    }

    NodePtr front() { assert( !empty() ); return ptr(head_); }
    NodeCPtr front() const { assert( !empty() ); return ptr(head_); }
    NodePtr back() { assert( !empty() ); return ptr(head_->prev); }
    NodeCPtr back() const { assert( !empty() ); return ptr(head_->prev); }

    // Mark as [[nodiscard]] so users don't confuse with clear()
    [[nodiscard]] bool empty() const {
        return head_ == nullptr;
    }

    void clear() {
        while( !empty() ) {
            pop_front();
        }
    }

    void push_front( NodePtr element ) {
        CompactIntrusiveList_Node *node = nodeptr(element.get());

        assert( node->next == nullptr );
        assert( node->prev == nullptr );

        if( empty() ) {
            node->next = node;
            node->prev = node;
        } else {
            node->next = head_;
            node->prev = head_->prev;

            node->next->prev = node;
            node->prev->next = node;
        }

        head_ = node;
        intrusive_ptr_add_ref( element.get() );
    }

    void push_back( NodePtr element ) {
        push_front( std::move(element) );

        head_ = head_->next;
    }

    void pop_front() {
        assert( !empty() );

        CompactIntrusiveList_Node *old_head = head_;
        head_ = old_head->next;

        assert( old_head->prev->next == old_head );
        assert( head_->prev == old_head );

        if( head_ == old_head ) {
            head_ = nullptr;
        } else {
            head_->prev = old_head->prev;
            head_->prev->next = head_;
        }

        old_head->next = nullptr;
        old_head->prev = nullptr;
        intrusive_ptr_release( ptr(old_head) );
    }

    void pop_back() {
        assert( !empty() );

        CompactIntrusiveList_Node *old_tail = head_->prev;

        assert( old_tail->next == head_ );
        assert( old_tail->prev->next == old_tail );

        if( head_ == old_tail ) {
            // List had one element
            head_ = nullptr;
        } else {
            head_->prev = old_tail->prev;
            head_->prev->next = head_;
        }

        old_tail->next = nullptr;
        old_tail->prev = nullptr;
        intrusive_ptr_release( ptr(old_tail) );
    }

    void erase( T *ptr ) {
        auto node = nodeptr( ptr );

        assert( node->next != nullptr );

        if( node->next == node ) {
            // List with just one element
            assert( node->prev == node );
            assert( head_ == node );    // Not a member of _our_ list

            head_ = nullptr;
        } else {
            node->next->prev = node->prev;
            node->prev->next = node->next;

            if( head_ == node )
                head_ = node->next;
        }

        node->next = nullptr;
        node->prev = nullptr;
    }

    void swap( CompactIntrusiveList &that ) noexcept {
        std::swap( head_, that.head_ );
    }

private:
    static T *ptr(CompactIntrusiveList_Node *node) {
        return reinterpret_cast<T *>( reinterpret_cast<std::byte *>(node) - NodeOffset );
    }

    static const T *ptr(const CompactIntrusiveList_Node *node) {
        return reinterpret_cast<const T *>( reinterpret_cast<const std::byte *>(node) - NodeOffset );
    }

    static CompactIntrusiveList_Node *nodeptr( T *element ) {
        return reinterpret_cast<CompactIntrusiveList_Node *>( reinterpret_cast<std::byte *>(element) + NodeOffset );
    }
};

} // namespace Fiberz::Containers

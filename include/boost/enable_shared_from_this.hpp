#ifndef BOOST_ENABLE_SHARED_FROM_THIS_HPP_INCLUDED
#define BOOST_ENABLE_SHARED_FROM_THIS_HPP_INCLUDED

//
//  enable_shared_from_this.hpp
//
//  Copyright (c) 2002 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  http://www.boost.org/libs/smart_ptr/enable_shared_from_this.html
//

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/config.hpp>

namespace boost
{

template<class T> class enable_shared_from_this
{
// dynamic cast to template type doesn't work in constructor, so we have
// to use lazy initialization
    void init_internal_shared_once() const
    {
        if( !owned() && _internal_shared_this.get() == 0 )
        {
            T * p = dynamic_cast<T *>(const_cast<enable_shared_from_this*>(this));
            _internal_shared_this = shared_ptr<T>( p, detail::sp_deleter_wrapper() );
            BOOST_ASSERT(_internal_shared_this.get() == this);
            _internal_weak_this = _internal_shared_this;
        }
    }

    bool owned() const
    {
        return _owned;
    }

    typedef T _internal_element_type; // for bcc 5.5.1
    mutable shared_ptr<_internal_element_type> _internal_shared_this;
    mutable weak_ptr<_internal_element_type> _internal_weak_this;
    mutable bool _owned;

protected:

    enable_shared_from_this():
      _owned(false)
    {
    }

    enable_shared_from_this(enable_shared_from_this const &):
      _owned(false)
    {
    }

    enable_shared_from_this & operator=(enable_shared_from_this const &)
    {
        return *this;
    }

// virtual destructor because we need a vtable for dynamic_cast from base to derived to work
    virtual ~enable_shared_from_this()
    {
// make sure no dangling shared_ptr objects were created by the
// user calling shared_from_this() but never passing ownership of the object
// to a shared_ptr.
        BOOST_ASSERT(owned() || _internal_shared_this.use_count() <= 1);
    }

public:

    shared_ptr<T> shared_from_this()
    {
        init_internal_shared_once();
        shared_ptr<T> p(_internal_weak_this);
        BOOST_ASSERT(p.get() == this);
        return p;
    }

    shared_ptr<T const> shared_from_this() const
    {
        init_internal_shared_once();
        shared_ptr<T const> p(_internal_weak_this);
        BOOST_ASSERT(p.get() == this);
        return p;
    }

    template<typename U>
    void _internal_accept_owner(shared_ptr<U> &owner) const
    {
        if( !_owned )
        {
            if( !_internal_shared_this )
            {
                T * p = dynamic_cast<T *>(const_cast<enable_shared_from_this*>(this));
                _internal_weak_this = shared_ptr<T>(owner, p);
            }else
            {
                detail::sp_deleter_wrapper * pd = get_deleter<detail::sp_deleter_wrapper>(_internal_shared_this);
                BOOST_ASSERT( pd != 0 );
                pd->set_deleter(owner);

                owner.reset( _internal_shared_this, owner.get() );
                _internal_shared_this.reset();
            }
            _owned = true;
        }
    }
};

template< class T, class Y > inline void sp_accept_owner( boost::shared_ptr<Y> * ptr, boost::enable_shared_from_this<T> const * pe )
{
    if( pe != 0 )
    {
        pe->_internal_accept_owner( *ptr );
    }
}

template< class T, class Y > inline void sp_accept_owner( boost::shared_ptr<Y> * ptr, boost::enable_shared_from_this<T> const * pe, void * /*pd*/ )
{
    if( pe != 0 )
    {
        pe->_internal_accept_owner( *ptr );
    }
}

template< class T, class Y > inline void sp_accept_owner( boost::shared_ptr<Y> * /*ptr*/, boost::enable_shared_from_this<T> const * /*pe*/, boost::detail::sp_deleter_wrapper * /*pd*/ )
{
}

} // namespace boost

#endif  // #ifndef BOOST_ENABLE_SHARED_FROM_THIS_HPP_INCLUDED

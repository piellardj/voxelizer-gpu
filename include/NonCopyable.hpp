#ifndef NONCOPYABLE_HPP_INCLUDED
#define NONCOPYABLE_HPP_INCLUDED


/**@brief Basic class for non copyable classes, typically OpenGL resource wrapper classes.
 Disables private copy constructor and assignment operator.
 To make a class non copyable, simply inherit from NonCopyable.
 */
class NonCopyable
{
    protected:
        NonCopyable() {};

    private:
        NonCopyable(NonCopyable const& o) = delete;

        NonCopyable& operator=(NonCopyable const& o) = delete;
};

#endif // NONCOPYABLE_HPP_INCLUDED

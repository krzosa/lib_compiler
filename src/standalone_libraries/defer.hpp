#ifndef FIRST_DEFER_HEADER
#define FIRST_DEFER_HEADER

template <typename T>
struct DEFER_ExitScope {
    T lambda;
    DEFER_ExitScope(T lambda) : lambda(lambda) {}
    ~DEFER_ExitScope() { lambda(); }
    DEFER_ExitScope(const DEFER_ExitScope &i) : lambda(i.lambda){};

  private:
    DEFER_ExitScope &operator=(const DEFER_ExitScope &);
};

class DEFER_ExitScopeHelp {
  public:
    template <typename T>
    DEFER_ExitScope<T> operator+(T t) { return t; }
};

#define DEFER_CONCAT_INTERNAL(x, y) x##y
#define DEFER_CONCAT(x, y) DEFER_CONCAT_INTERNAL(x, y)
#define defer const auto DEFER_CONCAT(defer__, __LINE__) = DEFER_ExitScopeHelp() + [&]()

#endif
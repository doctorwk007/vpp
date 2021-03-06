#include <iostream>
#include <vpp/core/boxNd.hh>

int main()
{
  using vpp::boxNd_iterator;
  using vpp::box2d;
  using vpp::vint2;


  box2d b(vint2(10,5), vint2(12,7));

  auto it = b.begin();

  assert(*it == vint2(10, 5));
  it.next();
  assert(*it == vint2(10, 6));
  it.next();
  assert(*it == vint2(10, 7));
  it.next();
  assert(*it == vint2(11, 5));
  it.next();
  assert(*it == vint2(11, 6));
  it.next();
  assert(*it == vint2(11, 7));
  it.next();
  assert(*it == vint2(12, 5));
  it.next();
  assert(*it == vint2(12, 6));
  it.next();
  assert(*it == vint2(12, 7));
  it.next();
  assert(it == b.end());

  vint2 ref[] = {
    vint2(10, 5),
    vint2(10, 6),
    vint2(10, 7),
    vint2(11, 5),
    vint2(11, 6),
    vint2(11, 7),
    vint2(12, 5),
    vint2(12, 6),
    vint2(12, 7)
  };

  int i = 0;
  for (auto p : b) assert(p == ref[i++]);
}

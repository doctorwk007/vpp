#ifndef VPP_PARALLEL_FOR_HH_
# define VPP_PARALLEL_FOR_HH_

# include <vpp/imageNd.hh>
# include <vpp/image2d.hh>
# include <vpp/boxNd.hh>
# include <vpp/tuple_utils.hh>

namespace vpp
{
  // Backends
  struct openmp {};
  struct pthreads {}; // Todo
  struct cpp_amp {}; // Todo

  template <typename D, typename B>
  class parallel_for_runner
  {
  public:

    parallel_for_runner(const D& domain) : domain_(domain) {}

    template <typename F>
    void operator<<(F fun);

  private:
    const D& domain_;
  };


  template <>
  template <typename F>
  void
  parallel_for_runner<box3d, openmp>::operator<<(F fun)
  {
    for (int s = domain_.p1()[0]; s <= domain_.p2()[0]; s++)
#pragma omp parallel for schedule(static, 2)
      for (int r = domain_.p1()[1]; r <= domain_.p2()[1]; r++)
        for (int c = domain_.p1()[2]; c <= domain_.p2()[2]; c++)
          fun(vint2(r, c));
  }

  template <>
  template <typename F>
  void
  parallel_for_runner<box2d, openmp>::operator<<(F fun)
  {
#pragma omp parallel for schedule(static, 4)
    for (int r = domain_.p1()[0]; r <= domain_.p2()[0]; r++)
      for (int c = domain_.p1()[1]; c <= domain_.p2()[1]; c++)
        fun(vint2(r, c));
  }

  template <>
  template <typename F>
  void
  parallel_for_runner<box1d, openmp>::operator<<(F fun)
  {
#pragma omp parallel for schedule(static, 50)
    for (int r = domain_.p1()[0]; r <= domain_.p2()[0]; r++)
      fun(vint1(r));
  }


  template <typename D>
  parallel_for_runner<D, openmp> parallel_for_openmp(D domain)
  {
    return parallel_for_runner<D, openmp>(domain);
  }

  template <typename D, typename B>
  parallel_for_runner<D, B> parallel_for(D domain, B backend)
  {
    return parallel_for_runner<D, B>(domain);
  }


  template <typename V, unsigned N>
  typename imageNd<V, N>::coord_type get_p1(imageNd<V, N>& img)
  {
    return img.domain().p1();
  }

  template <typename C, unsigned N, typename... PS>
  typename boxNd<N, C>::coord_type get_p1(boxNd<N, C>& box)
  {
    return box.p1();
  }

  template <typename V, unsigned N>
  typename imageNd<V, N>::coord_type get_p2(imageNd<V, N>& img)
  {
    return img.domain().p2();
  }

  template <typename C, unsigned N, typename... PS>
  typename boxNd<N, C>::coord_type get_p2(boxNd<N, C>& box)
  {
    return box.p2();
  }



  template <typename B, typename... Params>
  class parallel_for_pixel_wise_runner;

  template <typename... Params>
  class parallel_for_pixel_wise_runner<openmp, Params...>
  {
  public:

    parallel_for_pixel_wise_runner(std::tuple<Params...> t) : ranges_(t) {}

    template <typename F>
    void operator<<(F fun)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

      int start = p1[0];
      int end = p2[0];
#pragma omp parallel for schedule(static, 10)
      for (int r = start; r <= end; r++)
      {
        auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
            typedef typename std::remove_reference_t<decltype(range)>::row_iterator IT;
            return IT(vint2(r, p1[1]), range);
          });

        typedef typename std::remove_reference_t<decltype(std::get<0>(ranges_))>::row_iterator IT1;
        auto end0_ = IT1(vint2(r, p2[1] + 1), std::get<0>(ranges_));
        auto& cur0_ = std::get<0>(cur_);

        while (cur0_ != end0_)
        {
          internals::apply_args_star(cur_, fun);
          internals::tuple_map(cur_, [] (auto& it) { it.next(); });
        }
      }
    }

  private:
    std::tuple<Params...> ranges_;
  };

  template <typename... PS>
  parallel_for_pixel_wise_runner<openmp, PS...> pixel_wise(PS&&... params)
  {
    return parallel_for_pixel_wise_runner<openmp, PS...>(std::forward_as_tuple(params...));
  }


  template <typename B, typename... Params>
  class parallel_for_row_wise_runner;

  template <typename... Params>
  class parallel_for_row_wise_runner<openmp, Params...>
  {
  public:

    parallel_for_row_wise_runner(std::tuple<Params...> t) : ranges_(t) {}

    template <typename F>
    void operator<<(F fun)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

      int start = p1[0];
      int end = p2[0];
#pragma omp parallel for schedule(static, 10)
      for (int r = start; r <= end; r++)
      {
        auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
            typedef typename std::remove_reference_t<decltype(range)>::iterator IT;
            return IT(vint2(r, p1[1]), range);
          });

        internals::apply_args_star(cur_, fun);
      }
    }

  private:
    std::tuple<Params...> ranges_;
  };

  template <typename... PS>
  parallel_for_row_wise_runner<openmp, PS...> row_wise(PS&&... params)
  {
    return parallel_for_row_wise_runner<openmp, PS...>(std::forward_as_tuple(params...));
  }


  template <typename B, typename... Params>
  class parallel_for_col_wise_runner;

  template <typename... Params>
  class parallel_for_col_wise_runner<openmp, Params...>
  {
  public:

    parallel_for_col_wise_runner(std::tuple<Params...> t) : ranges_(t) {}

    template <typename F>
    void operator<<(F fun)
    {
      auto p1 = get_p1(std::get<0>(ranges_));
      auto p2 = get_p2(std::get<0>(ranges_));

      int start = p1[1];
      int end = p2[1];
#pragma omp parallel for schedule(static, 10)
      for (int c = start; c <= end; c++)
      {
        auto cur_ = internals::tuple_transform(ranges_, [&] (auto& range) {
            typedef typename std::remove_reference_t<decltype(range)>::iterator IT;
            return IT(vint2(p1[0], c), range);
          });

        internals::apply_args_star(cur_, fun);
      }
    }

  private:
    std::tuple<Params...> ranges_;
  };

  template <typename... PS>
  parallel_for_col_wise_runner<openmp, PS...> col_wise(PS&&... params)
  {
    return parallel_for_col_wise_runner<openmp, PS...>(std::forward_as_tuple(params...));
  }

};

#endif

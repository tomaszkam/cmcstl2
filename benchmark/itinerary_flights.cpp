// cmcstl2 - A concept-enabled C++ standard library
//
// Copyright Tomasz Kamiński 2016
// Copyright Eric Niebler 2018
//
// Use, modification and distribution is subject to the
// Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/caseycarter/cmcstl2

#include <vector>
#include <algorithm>
#include <random>
#include <array>
#include <benchmark/benchmark.h>
#include <experimental/ranges/range>
#include <experimental/ranges/algorithm>
#include <iostream>

namespace std::experimental::ranges
{
    template<class Ref>
    struct any_view
    {
        template<Range R>
          requires Same<iter_reference_t<iterator_t<R>>, Ref>
        any_view(R&&)
        {}
        std::add_pointer_t<Ref> begin() const { return 0; }
        std::add_pointer_t<Ref> end() const { return 0; }
    };
    template <Range R1, Range R2>
      requires CommonReference<iter_reference_t<iterator_t<R1>>,
                               iter_reference_t<iterator_t<R2>>>
    struct common_type<R1, R2>
    {
        using type =
            any_view<common_reference_t<iter_reference_t<iterator_t<R1>>,
                                        iter_reference_t<iterator_t<R2>>>>;
    };
}

namespace ranges = std::experimental::ranges;

struct Date 
{
    int v;
};

bool operator==(Date d1, Date d2) { return d1.v == d2.v; }
bool operator!=(Date t1, Date t2) { return !(t1 == t2); }

bool operator<(Date d1, Date d2) { return d1.v < d2.v; }
bool operator>(Date t1, Date t2) { return t1 < t2; }
bool operator>=(Date t1, Date t2) { return !(t1 < t2); }
bool operator<=(Date t1, Date t2) { return !(t2 < t1); }

struct Flight
{
    Date d;
    Date departure_date() const { return d; }
};

struct Leg
{
    std::vector<Flight> f;
    std::vector<Flight> const& flights() const { return f; }
};

struct Itinerary
{
    std::vector<Leg> l;
    std::vector<Leg> const& legs() const { return l; }
};

struct FLightDepartureLess
{ 
    bool operator()(Date d1, Date d2) const
    { return d1 < d2; }

    bool operator()(Date d, Flight const& f) const
    { return d < f.departure_date(); }

    bool operator()(Flight const& f, Date d) const
    { return  f.departure_date() < d; }

    bool operator()(Flight const& f1, Flight const& f2) const
    { return f1.departure_date() < f2.departure_date(); }
};


// Do not break law of useful dates
template<typename I1, typename I2, typename C>
int lexicographical_compare_int(I1 f1, I1 l1, I2 f2, I2 l2, C c)
{
  while (f1 != l1)
  {
     if (f2 == l2)
       return 1;

     if (c(*f1, *f2))
       return -1;
     else if (c(*f2, *f1))
       return 1;
    
     ++f1; ++f2;
  }

  if (f2 != l2)
    return -1;

  return 0;
}

template<typename FlightLess>
struct FlightsComparator
{
   FlightsComparator() = default;
   explicit FlightsComparator(FlightLess fl) : flightLess(std::move(fl)) {}

   template<typename T>
   bool operator()(std::vector<T> const& d1, std::vector<T> const& d2)
   {  
      return std::lexicographical_compare(d1.begin(), d1.end(), 
                                          d2.begin(), d2.end(), 
                                          flightLess);
   }

   template<typename T>
   bool operator()(std::vector<T> const& ds, Itinerary const& i)
   { return compareFlatenned(i, ds) > 0; }

   template<typename T>
   bool operator()(Itinerary const& i, std::vector<T> const& ds)
   { return compareFlatenned(i, ds) < 0; }

   bool operator()(Itinerary const& i1, Itinerary const& i2)
   { 
      auto const legsLess = [&](Leg const& l1, Leg const& l2)
      {
         return std::lexicographical_compare(l1.flights().begin(), l1.flights().end(),
                                             l2.flights().begin(), l2.flights().end(),
                                             flightLess);
      };
      return std::lexicographical_compare(i1.legs().begin(), i1.legs().end(),
                                          i2.legs().begin(), i2.legs().end(),
                                          legsLess);
   }


private:
   template<typename T>
   int compareFlatenned(Itinerary const& i, std::vector<T> const& ds)
   {
      auto beg = ds.begin();
      for (Leg const& l : i.legs())
      {
         if (std::distance(beg, ds.end()) < l.flights().size())
           return -1;

         auto const end = beg + l.flights().size();
         int const result = 
           lexicographical_compare_int(
             l.flights().begin(), l.flights().end(),
             beg, end,
             flightLess);

         if (result)
           return result;
          
         beg = end;
      }
 
      if (beg != ds.end())
        return -1;
     
      return 0;
   }
  
   FlightLess flightLess;
};

class ItineraryFixture : public ::benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& st)
    {
        std::size_t const elements = st.range(0);
        std::size_t const legs = st.range(1); 

        std::mt19937 gen;
        std::uniform_int_distribution<int> dist(0, 30);
        std::uniform_int_distribution<int> flights(1, 3);

        dates.resize(legs * flights(gen));
        for (Date& d : dates)
          d.v = dist(gen);

        itineraries.resize(elements);
        for (Itinerary& itin : itineraries)
        {
           itin.l.resize(legs);
           for (Leg& leg : itin.l)
           {
              leg.f.resize(flights(gen));
              for (Flight& flight : leg.f)
                flight.d.v = dist(gen);
           }
        }
        std::sort(itineraries.begin(), itineraries.end(), 
                  FlightsComparator(FLightDepartureLess())); 
    }

    void TearDown(const ::benchmark::State&)
    {
        itineraries.clear();
        dates.clear();
    }

   std::vector<Itinerary> itineraries; 
   std::vector<Date> dates;
};

BENCHMARK_DEFINE_F(ItineraryFixture, STL1)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(
            std::equal_range(itineraries.begin(), itineraries.end(),
                             dates, FlightsComparator(FLightDepartureLess())));
    }
}
BENCHMARK_REGISTER_F(ItineraryFixture, STL1)
  -> Ranges({{1<<10, 1<<20}, {1,2}});

inline constexpr auto toDate = [](const Flight& f)
{
    return f.departure_date();
};

inline constexpr auto toFlightsDates = [](const Leg& l)
{
    return l.flights() | ranges::view::transform(toDate);
};

inline constexpr auto toDates = [](Itinerary& i)
{
    return ranges::view::join(i.legs() | ranges::view::transform(toFlightsDates));
};

BENCHMARK_DEFINE_F(ItineraryFixture, STL2)(benchmark::State& state)
{
  
   while (state.KeepRunning())
   {
        benchmark::DoNotOptimize(
            ranges::equal_range(
                itineraries,
                dates,
                [](auto&& r1, auto&& r2) { return ranges::lexicographical_compare(r1, r2); },
                toDates));
    }
}
BENCHMARK_REGISTER_F(ItineraryFixture, STL2)
  -> Ranges({{1<<10, 1<<20}, {1,2}});


BENCHMARK_MAIN();

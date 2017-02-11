/**
 * @file concept_checks.h
 * @todo Write header docs for concept_checks.h.h
 * @author Andrzej Ciarkowski <andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_CONCEPT_CHECKS_H_INCLUDED
#define DSP_CONCEPT_CHECKS_H_INCLUDED

#include <dsp++/config.h>

#if DSP_BOOST_CONCEPT_CHECKS_DISABLED

# define DSP_CONCEPT_ASSERT(...) \
	typedef void dsp_detail_concept_check_ ## __LINE__ ## __

# define DSP_CONCEPT_REQUIRES(models, result) result

#else // !DSP_BOOST_CONCEPT_CHECKS_DISABLED
# include <boost/concept_check.hpp>
# include <boost/concept/requires.hpp>
# define DSP_CONCEPT_ASSERT BOOST_CONCEPT_ASSERT
# define DSP_CONCEPT_REQUIRES BOOST_CONCEPT_REQUIRES
#endif // !DSP_BOOST_CONCEPT_CHECKS_DISABLED

#endif /* DSP_CONCEPT_CHECKS_H_INCLUDED */

#include "reductions.h"

const int autoconstant = 524267083;

struct autolink
{
  uint32_t d; // degree of the polynomial
  uint32_t stride_shift;
};

template <bool is_learn>
void predict_or_learn(autolink& b, LEARNER::single_learner& base, example& ec)
{
  base.predict(ec);
  float base_pred = ec.pred.scalar;
  // add features of label
  ec.indices.push_back(autolink_namespace);
  features& fs = ec.feature_space[autolink_namespace];
  for (size_t i = 0; i < b.d; i++)
    if (base_pred != 0.)
    {
      fs.push_back(base_pred, autoconstant + (i << b.stride_shift));
      base_pred *= ec.pred.scalar;
    }
  ec.total_sum_feat_sq += fs.sum_feat_sq;

  if (is_learn)
    base.learn(ec);
  else
    base.predict(ec);

  ec.total_sum_feat_sq -= fs.sum_feat_sq;
  fs.erase();
  ec.indices.pop();
}

LEARNER::base_learner* autolink_setup(arguments& arg)
{
  free_ptr<autolink> data = scoped_calloc_or_throw<autolink>();
  if (arg.new_options("Autolink")
      .critical("autolink", data->d, "create link function with polynomial d").missing())
    return nullptr;

  data->stride_shift = arg.all->weights.stride_shift();

  LEARNER::learner<autolink,example>& ret =
    init_learner(data, setup_base(arg), predict_or_learn<true>, predict_or_learn<false>);

  return make_base(ret);
}

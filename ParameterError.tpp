
/*
    -------------------------------
        ParameterError methods
    -------------------------------
*/

template <typename ParamType>
ParameterError<ParamType>::ParameterError(ParamType param_src, const std::string &msg_src)
{
    param = param_src;
    msg = msg_src;
}

template <typename ParamType>
void
ParameterError<ParamType>::print(std::ostream &out) const
{
    out << PARAM_ERR_MSG << msg << ": " << param;
}

template <typename ParamType>
std::string
ParameterError<ParamType>::get_msg() const
{
    return PARAM_ERR_MSG + msg;
}

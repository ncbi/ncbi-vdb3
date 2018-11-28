#include <string>

/**
 * \brief namespace for all things NCBI
 * Here is a more detailed description of NCBI
 */
namespace NCBI /*!< Detailed description after the member */
{
    /**
     * \brief namespace for VDB3 stuff
     * Here is a more detailed description of NCBI::VDB3
     */
    namespace VDB3 /**< Detailed description after the member */
    {
        /**
         * \brief TestMessage: a Doxygen description in the JavaDoc style
         */
        typedef std::string PlatformMessage; //!< Detailed description after the member
                                             //!<

        /*!
         * \brief HelloMsg: a Doxygen description in the Qt style
         */
        PlatformMessage HelloMsg(); ///< Detailed description after the member
                                    ///<
    }
}

/*! \file encryption_interface.h
 * \brief EncryptionInterface class interface.
 *
 * Class description.
 *
 */


#pragma once

/*! \class EncryptionInterface
 * \brief Some briefing
 */
class EncryptionInterface {
    EncryptionInterface(const EncryptionInterface&) = delete;
    EncryptionInterface(EncryptionInterface&&) = delete;
    EncryptionInterface operator=(const EncryptionInterface&) = delete;
    EncryptionInterface operator=(EncryptionInterface&&) = delete;
public:

    //! \brief default constructor.
    virtual void setKey() = 0;

    //! \brief virtual destructor.
    virtual ~EncryptionInterface();
};

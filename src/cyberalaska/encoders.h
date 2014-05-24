/*
 * encoders.h
 *
 *  Created on: Mar 8, 2014
 *      Author: shaun
 */

#ifndef ENCODERS_H_
#define ENCODERS_H_

namespace cyberalaska {
    class encoder_t : public sensor_t {
    public:
        // Right and left encoder values
        short right;
        short left;

        encoder_t(const metadata_sensor &metadata_)
            :sensor_t(metadata_), right(0), left(0) {}
    };

};

#endif /* ENCODERS_H_ */

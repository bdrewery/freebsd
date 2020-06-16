/*
 * \file       trc_data_rawframe_in_i.h
 * \brief      OpenCSD : 
 * 
 * \copyright  Copyright (c) 2015, ARM Limited. All Rights Reserved.
 */


/* 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution. 
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 * may be used to endorse or promote products derived from this software without 
 * specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */ 

#ifndef ARM_TRCDATA_RAWFRAME_IN_I_H_INCLUDED
#define ARM_TRCDATA_RAWFRAME_IN_I_H_INCLUDED

#include "opencsd/ocsd_if_types.h"

/*!
 * @class ITrcRawFrameIn   
 * 
 * @brief Interface to monitor the raw frame decode progress..
 *
 * @ingroup ocsd_interfaces
 *
 * This interface allows a program to monitor the contents of the CoreSight frames passing through the 
 * frame deformatter.
 *
 *
 */
class ITrcRawFrameIn {
public:
    ITrcRawFrameIn() {};  /**< Default constructor. */
    virtual ~ITrcRawFrameIn() {};  /**< Default destructor. */
    
    /*!
     * Interface to monitor CoreSight frame data. Output as blocks of data.
     *
     * @param op : Data path operation.
     * @param index : Byte index of start of pDataBlock data as offset from start of captured data. May be zero for none-data operation 
     * @param frame_element : Type of frame element being output.
     * @param dataBlockSize : size of frame element.
     * @param *pDataBlock : pointer to frame data.
     * @param traceID : Trace ID when element type ID data.
     *
     * @return ocsd_err_t  : Standard library erroc code. Monitor only, not on data path.
     */
    virtual ocsd_err_t TraceRawFrameIn(  const ocsd_datapath_op_t op, 
                                          const ocsd_trc_index_t index, 
                                          const ocsd_rawframe_elem_t frame_element, 
                                          const int dataBlockSize, 
                                          const uint8_t *pDataBlock,
                                          const uint8_t traceID) = 0;
};


#endif // ARM_TRCDATA_RAWFRAME_IN_I_H_INCLUDED

/* End of File trc_data_rawframe_in_i.h */

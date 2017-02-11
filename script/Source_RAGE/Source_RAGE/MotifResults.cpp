/** 
 * Copyright 2010 Dror Marcus. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY DROR MARCUS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DROR MARCUS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Dror Marcus.
 */
#include "MotifResults.h"

const char* MotifResults::MOTIF_TYPES_NAMES[MotifResults::MOTIF_TYPES_NUM] = {
		"PATH3_EDGE", 
		"PATH3_INNR", 
		"TRIANGLE", 
		"PATH4_EDGE", 
		"PATH4_INNR", 
		"CYCLE4", 
		"TAILED_TGL_TAIL", 
		"TAILED_TGL_CNTR", 
		"TAILED_TGL_REG",
		"CYC_CHORD_REG",
		"CYC_CHORD_ONCHORD",
		"CLAW_CNTR",
		"CLAW_EDGE",
		"CLIQUE4"
	};

const int MotifResults::URI_MOTIF_TYPES_ID_CONVERTER[MotifResults::MOTIF_TYPES_NUM][2] =
{
	{78,1}, //"PATH3_EDGE", 
	{78,0}, //"PATH3_INNR", 
	{238,0}, //"TRIANGLE", 
	{4698,1}, //	"PATH4_EDGE", 
	{4698,0}, //	"PATH4_INNR", 
	{13260,0}, //"CYCLE4", 
	{4958,2}, //"TAILED_TGL_TAIL", 
	{4958,0}, //"TAILED_TGL_CNTR", 
	{4958,1}, //"TAILED_TGL_REG",
	{13278,1}, //	"CYC_CHORD_REG",
	{13278,0}, //"CYC_CHORD_ONCHORD",
	{4382,0}, //"CLAW_CNTR",
	{4382,1}, //"CLAW_EDGE",
	{31710,0}, //"CLIQUE4"
};

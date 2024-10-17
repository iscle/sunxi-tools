/* Automatically generated, do not edit! */

static void
aw_fel_remotefunc_prepare_sunxi_mmc_trans_data(feldev_handle *dev,
                                               uint32_t              buff,
                                               uint32_t              blocks,
                                               uint32_t              blocksize,
                                               uint32_t              reading,
                                               uint32_t              rsp_busy,
                                               uint32_t              mmc_rintsts_reg,
                                               uint32_t              mmc_status_reg,
                                               uint32_t              mmc_ctrl_reg,
                                               uint32_t              mmc_fifo_reg)
{
	static uint8_t arm_code[] = {
		0xf0, 0x4f, 0x2d, 0xe9, /*    0:    push     {r4, r5, r6, r7, r8, r9, sl, fp, lr} */
		0x24, 0x40, 0x8d, 0xe2, /*    4:    add      r4, sp, #36	@ 0x24                 */
		0x70, 0x10, 0x94, 0xe8, /*    8:    ldm      r4, {r4, r5, r6, ip}               */
		0x91, 0x02, 0x02, 0xe0, /*    c:    mul      r2, r1, r2                         */
		0x00, 0x00, 0x53, 0xe3, /*   10:    cmp      r3, #0                             */
		0x22, 0xe1, 0xa0, 0xe1, /*   14:    lsr      lr, r2, #2                         */
		0x00, 0x20, 0x9c, 0xe5, /*   18:    ldr      r2, [ip]                           */
		0x34, 0x70, 0x9d, 0xe5, /*   1c:    ldr      r7, [sp, #52]	@ 0x34               */
		0x02, 0x21, 0x82, 0xe3, /*   20:    orr      r2, r2, #-2147483648	@ 0x80000000  */
		0xbc, 0x90, 0x9f, 0xe5, /*   24:    ldr      r9, [pc, #188]	@ e8 <sunxi_mmc_trans_data+0xe8> */
		0x00, 0x20, 0x8c, 0xe5, /*   28:    str      r2, [ip]                           */
		0x08, 0x80, 0xa0, 0x03, /*   2c:    moveq    r8, #8                             */
		0x04, 0x80, 0xa0, 0x13, /*   30:    movne    r8, #4                             */
		0x00, 0x20, 0xa0, 0xe3, /*   34:    mov      r2, #0                             */
		0x02, 0x00, 0x5e, 0xe1, /*   38:    cmp      lr, r2                             */
		0x11, 0x00, 0x00, 0x8a, /*   3c:    bhi      88 <sunxi_mmc_trans_data+0x88>     */
		0xa4, 0x30, 0x9f, 0xe5, /*   40:    ldr      r3, [pc, #164]	@ ec <sunxi_mmc_trans_data+0xec> */
		0x00, 0x00, 0x95, 0xe5, /*   44:    ldr      r0, [r5]                           */
		0x03, 0x00, 0x10, 0xe1, /*   48:    tst      r0, r3                             */
		0xf0, 0x8f, 0xbd, 0x18, /*   4c:    popne    {r4, r5, r6, r7, r8, r9, sl, fp, pc} */
		0x04, 0x00, 0x10, 0xe3, /*   50:    tst      r0, #4                             */
		0xfa, 0xff, 0xff, 0x0a, /*   54:    beq      44 <sunxi_mmc_trans_data+0x44>     */
		0x01, 0x00, 0x51, 0xe3, /*   58:    cmp      r1, #1                             */
		0x01, 0x29, 0xa0, 0x83, /*   5c:    movhi    r2, #16384	@ 0x4000                */
		0x08, 0x20, 0xa0, 0x93, /*   60:    movls    r2, #8                             */
		0x00, 0x00, 0x95, 0xe5, /*   64:    ldr      r0, [r5]                           */
		0x03, 0x00, 0x10, 0xe1, /*   68:    tst      r0, r3                             */
		0xf0, 0x8f, 0xbd, 0x18, /*   6c:    popne    {r4, r5, r6, r7, r8, r9, sl, fp, pc} */
		0x00, 0x00, 0x12, 0xe1, /*   70:    tst      r2, r0                             */
		0xfa, 0xff, 0xff, 0x0a, /*   74:    beq      64 <sunxi_mmc_trans_data+0x64>     */
		0x00, 0x00, 0x54, 0xe3, /*   78:    cmp      r4, #0                             */
		0x15, 0x00, 0x00, 0x1a, /*   7c:    bne      d8 <sunxi_mmc_trans_data+0xd8>     */
		0x04, 0x00, 0xa0, 0xe1, /*   80:    mov      r0, r4                             */
		0xf0, 0x8f, 0xbd, 0xe8, /*   84:    pop      {r4, r5, r6, r7, r8, r9, sl, fp, pc} */
		0x00, 0xc0, 0x96, 0xe5, /*   88:    ldr      ip, [r6]                           */
		0x08, 0x00, 0x1c, 0xe1, /*   8c:    tst      ip, r8                             */
		0xfc, 0xff, 0xff, 0x1a, /*   90:    bne      88 <sunxi_mmc_trans_data+0x88>     */
		0x00, 0x00, 0x53, 0xe3, /*   94:    cmp      r3, #0                             */
		0x02, 0xc1, 0x90, 0x07, /*   98:    ldreq    ip, [r0, r2, lsl #2]               */
		0x00, 0xc0, 0x87, 0x05, /*   9c:    streq    ip, [r7]                           */
		0x01, 0x20, 0x82, 0x02, /*   a0:    addeq    r2, r2, #1                         */
		0xe3, 0xff, 0xff, 0x0a, /*   a4:    beq      38 <sunxi_mmc_trans_data+0x38>     */
		0xac, 0xa8, 0x19, 0xe0, /*   a8:    ands     sl, r9, ip, lsr #17                */
		0x0c, 0xc1, 0xa0, 0x01, /*   ac:    lsleq    ip, ip, #2                         */
		0x20, 0xa0, 0x0c, 0x02, /*   b0:    andeq    sl, ip, #32                        */
		0x02, 0xc1, 0x80, 0xe0, /*   b4:    add      ip, r0, r2, lsl #2                 */
		0x0a, 0x20, 0x82, 0xe0, /*   b8:    add      r2, r2, sl                         */
		0x02, 0xa1, 0x80, 0xe0, /*   bc:    add      sl, r0, r2, lsl #2                 */
		0x0a, 0x00, 0x5c, 0xe1, /*   c0:    cmp      ip, sl                             */
		0x00, 0x00, 0x00, 0x1a, /*   c4:    bne      cc <sunxi_mmc_trans_data+0xcc>     */
		0xda, 0xff, 0xff, 0xea, /*   c8:    b        38 <sunxi_mmc_trans_data+0x38>     */
		0x00, 0xb0, 0x97, 0xe5, /*   cc:    ldr      fp, [r7]                           */
		0x04, 0xb0, 0x8c, 0xe4, /*   d0:    str      fp, [ip], #4                       */
		0xf9, 0xff, 0xff, 0xea, /*   d4:    b        c0 <sunxi_mmc_trans_data+0xc0>     */
		0x00, 0x00, 0x96, 0xe5, /*   d8:    ldr      r0, [r6]                           */
		0x02, 0x0c, 0x10, 0xe2, /*   dc:    ands     r0, r0, #512	@ 0x200               */
		0xfc, 0xff, 0xff, 0x1a, /*   e0:    bne      d8 <sunxi_mmc_trans_data+0xd8>     */
		0xf0, 0x8f, 0xbd, 0xe8, /*   e4:    pop      {r4, r5, r6, r7, r8, r9, sl, fp, pc} */
		0xff, 0x3f, 0x00, 0x00, /*   e8:    .word    0x00003fff                         */
		0xc2, 0xbf, 0x00, 0x00, /*   ec:    .word    0x0000bfc2                         */
	};
	uint32_t args[] = {
		buff,
		blocks,
		blocksize,
		reading,
		rsp_busy,
		mmc_rintsts_reg,
		mmc_status_reg,
		mmc_ctrl_reg,
		mmc_fifo_reg
	};
	aw_fel_remotefunc_prepare(dev, 36, arm_code, sizeof(arm_code), 9, args);
}

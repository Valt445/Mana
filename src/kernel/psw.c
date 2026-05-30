#include "psw.h"
#include "aarch64.h"
#include <stdint.h>
// borowwed from https://github.com/NienfengYao/armv8-bare-metal
/** Disable interrupt at CPU level
    @ param [in] pswp Processor status word return area before interrupt
disabled
 */
void psw_disable_and_save_interrupt(psw_t *pswp) {
  psw_t psw;

  __save_psw(psw);
  psw_disable_interrupt();
  *pswp = psw;
}

/** Restore interrupt status at CPU level
    @ param [in] pswp Processor status word return area
 */
void psw_restore_interrupt(psw_t *pswp) {
  psw_t psw;

  psw = *pswp;
  __restore_psw(psw);
}

#ifndef __KERNEL__
#  define __KERNEL__
#endif

#include "config.h"
#include <linux/slab.h>
#include "devstruct.h"
#include "tmknllio.h"

void load_buf_data_new(u16 max_packet_size, u8 * buf, u16 * buf_data,
                       u16 count_buf_in, u16 * pointer);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

unsigned int Block_out_in(struct tmk1553busb * dev, u16 *buf_out, u16 *buf_in)
{
  int Result;
  unsigned int err = 0;
  u16 max_packet_size = dev->ep2_maxsize;
  u8 * buffer;
  u16 count_buffer_out = 0; //Pointer in USB buffer
  u16 count_buf_out = 0; //USB soft out pointer
  u16 count_buffer_in = 0;
  u16 count_buf_in = 0; //USB soft in pointer
  u16 pointer = 0; //pointer for load_buf_data_new
  unsigned char command;
  u8 incorrect_command = FALSE;
  u8 out_enable = TRUE;
  u8 in_enable = FALSE;
  u16 Index;
  u8 buff_ovf = FALSE;
  int count;

  buffer = kmalloc (sizeof(u8) * max_packet_size, GFP_KERNEL);
  if (buffer == NULL)
  {
#ifdef TMK1553BUSB_DEBUG
    printk(KERN_INFO "tmk1553busb: block_out_in: Out of memory!\n");
#endif
    return 1;
  }

  for(; (buf_out[count_buf_out] != 0xFFFF) && (incorrect_command == FALSE);)
  {
    command = buf_out[count_buf_out];
#ifdef PARSER_DEBUG
    printk(KERN_INFO "tmk1553busb: block_out_in: command %d\n", command);
#endif
    switch(command) //Command select
    {
      case 0: //Write RG
        if(max_packet_size >= count_buffer_out + 5) //Check free space
        {
          out_enable = TRUE; //Enable output transaction
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command
          buffer[count_buffer_out + 1] = (buf_out[count_buf_out + 1] << 2) | 7;//write address
          buffer[count_buffer_out + 2] = buf_out[count_buf_out + 2] >> 8;//write MSB data
          buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] & 0xFF;//write LSB data
          count_buffer_out += 4;
          count_buf_out += 3;
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 1: //Read RG
        if((max_packet_size > count_buffer_out + 3) &&
           (max_packet_size > count_buffer_in + 3))
        {
          out_enable = TRUE;
          in_enable = TRUE;
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command
          buf_in[count_buf_in] = command;

          buffer[count_buffer_out + 1] = (buf_out[count_buf_out + 1] << 2) | 7;
          buf_in[count_buf_in + 1] = buf_out[count_buf_out + 1];

          count_buffer_out += 2;
          count_buf_out += 2;
          count_buf_in += 3;
          count_buffer_in += 2;
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 2: //Write mem
        if(max_packet_size <= (buf_out[count_buf_out + 1] << 1)) //Check free space
        {
          err = 3;
          goto error;
        }

        if(max_packet_size >=
           count_buffer_out + 6 + (buf_out[count_buf_out + 1] << 1)) //Check free space
        {
          out_enable = TRUE; //Enable output transaction
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command
          buffer[count_buffer_out + 1] = buf_out[count_buf_out + 1] >> 8;//write repeat counter
          buffer[count_buffer_out + 2] = buf_out[count_buf_out + 1] & 0xFF;
          buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] >> 8;//write MSB address
          buffer[count_buffer_out + 4] = buf_out[count_buf_out + 2] & 0xFF;//write LSB address

          count = buf_out[count_buf_out + 1];
          count_buffer_out += 5;
          count_buf_out += 3;

          for(Index = 0; Index < count; count_buffer_out += 2, count_buf_out++, Index++)
          {
            buffer[count_buffer_out] = buf_out[count_buf_out] >> 8;
            buffer[count_buffer_out + 1] = buf_out[count_buf_out] & 0xFF;
          }

        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 3: //read mem
        if(max_packet_size < (buf_out[count_buf_out + 1] << 1)) //Check free space
        {
          err = 3;
          goto error;
        }

        if((max_packet_size >= count_buffer_out + 6) &&
           (max_packet_size >=
            count_buffer_in + (buf_out[count_buf_out + 1] << 1)))
        {
          out_enable = TRUE;
          in_enable = TRUE;
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command
          buf_in[count_buf_in] = command;

          buffer[count_buffer_out + 1] = buf_out[count_buf_out + 1] >> 8;
          buffer[count_buffer_out + 2] = buf_out[count_buf_out + 1] & 0xFF;
          buf_in[count_buf_in + 1] = buf_out[count_buf_out + 1];

          buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] >> 8;
          buffer[count_buffer_out + 4] = buf_out[count_buf_out + 2] & 0xFF;
          buf_in[count_buf_in + 2] = buf_out[count_buf_out + 2];

          count_buf_in += 3 + buf_out[count_buf_out + 1];
          count_buffer_in += buf_out[count_buf_out + 1] << 1;

          count_buffer_out += 5;
          count_buf_out += 3;
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 4:
      case 5: //Read/Mod/Write RG
        if(max_packet_size > count_buffer_out + 7) //Check free space
        {
          out_enable = TRUE; //Enable output transaction
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command
          buffer[count_buffer_out+1] = (buf_out[count_buf_out+1] << 2) | 7;//write address

          buffer[count_buffer_out + 2] = buf_out[count_buf_out + 2] >> 8;//write MSB data
          buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] & 0xFF;//write LSB data
          buffer[count_buffer_out + 4] = buf_out[count_buf_out + 3] >> 8;//write MSB data
          buffer[count_buffer_out + 5] = buf_out[count_buf_out + 3] & 0xFF;//write LSB data
          count_buffer_out += 6;
          count_buf_out += 4;
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 6:
      case 7: //Read/Mod/Write Mem
        if(max_packet_size > count_buffer_out + 10) //Check free space
        {
          out_enable = TRUE; //Enable output transaction
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command

          buffer[count_buffer_out + 1] = buf_out[count_buf_out+1] >> 8;
          buffer[count_buffer_out + 2] = buf_out[count_buf_out+1] & 0xFF;
          buffer[count_buffer_out + 3] = buf_out[count_buf_out+2] >> 8;
          buffer[count_buffer_out + 4] = buf_out[count_buf_out+2] & 0xFF;
          buffer[count_buffer_out + 5] = buf_out[count_buf_out+3] >> 8;
          buffer[count_buffer_out + 6] = buf_out[count_buf_out+3] & 0xFF;
          buffer[count_buffer_out + 7] = buf_out[count_buf_out+4] >> 8;
          buffer[count_buffer_out + 8] = buf_out[count_buf_out+4] & 0xFF;

          count_buffer_out += 9;
          count_buf_out += 5;
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 10: //Write Param
        if((buf_out[count_buf_out + 1] < 10) &&
           (max_packet_size > count_buffer_out + 5)) //Check free space
        {
          out_enable = TRUE; //Enable output transaction
          buffer[count_buffer_out] = buf_out[count_buf_out];//write command to USB buffer
          buffer[count_buffer_out + 1] = buf_out[count_buf_out + 1];//write var num to USB buffer
          buffer[count_buffer_out + 2] = buf_out[count_buf_out + 2] >> 8;//write MSB data register to USB buffer
          buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] & 0xFF;//write LSB data register to USB buffer
          count_buffer_out += 4;
          count_buf_out += 3;
        }
        else if((buf_out[count_buf_out + 1] > 9) &&
                (buf_out[count_buf_out + 1] < 13)) //if var num = 10,11,12
        {
          if(max_packet_size <= 6 + (buf_out[count_buf_out + 2] << 1)) //Check free space
          {
            err = 3;
            goto error;
          }

          if(max_packet_size >
             count_buffer_out + (buf_out[count_buf_out + 2] << 1) + 7) //Check free space
          {
            out_enable = TRUE; //Enable output transaction
            buffer[count_buffer_out] = buf_out[count_buf_out];//write command
            buffer[count_buffer_out + 1] = buf_out[count_buf_out + 1];//write var num
            buffer[count_buffer_out + 2] = buf_out[count_buf_out + 2] >> 8;
            buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] & 0xFF;//write MSB address
            buffer[count_buffer_out + 4] = buf_out[count_buf_out + 3] >> 8;//write MSB address
            buffer[count_buffer_out + 5] = buf_out[count_buf_out + 3] & 0xFF;//write LSB address

            count = buf_out[count_buf_out + 2];
            count_buffer_out += 6;
            count_buf_out += 4;
            for(Index = 0; Index < count;
                count_buffer_out += 2, count_buf_out++, Index++)
            {
              buffer[count_buffer_out] = buf_out[count_buf_out] >> 8;
              buffer[count_buffer_out + 1] = buf_out[count_buf_out] & 0xFF;
            }
          }
          else
          {
            buffer[count_buffer_out] = 0xFF;
            buff_ovf = TRUE;
          }
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      case 11:
        if((buf_out[count_buf_out + 1] < 10) &&
           (max_packet_size > count_buffer_out + 3) &&
           (max_packet_size > count_buffer_in + 3))
        {
          out_enable = TRUE;
          in_enable = TRUE;

          buffer[count_buffer_out] = buf_out[count_buf_out];//write command
          buf_in[count_buf_in] = command;

          buffer[count_buffer_out + 1] = buf_out[count_buf_out + 1];
          buf_in[count_buf_in + 1] = buf_out[count_buf_out + 1];

          count_buf_in += 3;
          count_buffer_in += 2;
          count_buffer_out += 2;
          count_buf_out += 2;
        }
        else if((buf_out[count_buf_out + 1] > 9) &&
                (buf_out[count_buf_out + 1] < 13)) //if var num = 10,11,12
        {
          if(max_packet_size <= (buf_out[count_buf_out + 2] << 1)) //Check free space
          {
            err = 3;
            goto error;
          }

          if((max_packet_size > count_buffer_out + 7) &&
             (max_packet_size >
              count_buffer_in + (buf_out[count_buf_out + 2] << 1) + 1)) //Check free space
          {
            out_enable = TRUE;
            in_enable = TRUE;

            buffer[count_buffer_out] = buf_out[count_buf_out];//write command to USB buffer
            buf_in[count_buf_in] = command;

            buffer[count_buffer_out + 1] = buf_out[count_buf_out + 1];
            buf_in[count_buf_in + 1] = buf_out[count_buf_out + 1];

            buffer[count_buffer_out + 2] = buf_out[count_buf_out + 2] >> 8;
            buffer[count_buffer_out + 3] = buf_out[count_buf_out + 2] & 0xFF;
            buf_in[count_buf_in + 2] = buf_out[count_buf_out + 2];

            buffer[count_buffer_out + 4] = buf_out[count_buf_out + 3] >> 8;
            buffer[count_buffer_out + 5] = buf_out[count_buf_out + 3] & 0xFF;
            buf_in[count_buf_in + 3] = buf_out[count_buf_out + 3];

            count_buffer_in += buf_out[count_buf_out + 2] << 1;
            count_buf_in += (4 + buf_out[count_buf_out + 2]);
            count_buffer_out += 6;
            count_buf_out += 4;
          }
          else
          {
            buffer[count_buffer_out] = 0xFF;
            buff_ovf = TRUE;
          }
        }
        else
        {
          buffer[count_buffer_out] = 0xFF;
          buff_ovf = TRUE;
        }
      break;
      default:
        err = 3;
        goto error;
    } //Switch

    if((buff_ovf == TRUE) && (incorrect_command == FALSE))
    {
      buffer[count_buffer_out] = 0xFF;
      buff_ovf = FALSE;
      count_buffer_in = 0;
      count_buffer_out = 0;
      out_enable = FALSE;

      Result = usb_bulk_msg(dev->udev,
                            usb_sndbulkpipe (dev->udev,
                                             dev->ep2_address),
                            buffer,
                            max_packet_size,
                            &count,
                            HZ*10);

#ifdef PARSER_DEBUG
      printk(KERN_INFO "tmk1553busb: block_out_in: ofwrite %d\n", Result);
#endif

      if(Result)
      {
        err = 1;
        goto error;
      }

      if(in_enable)
      {
        in_enable=FALSE;
        buf_in[count_buf_in]=0;

        Result = usb_bulk_msg(dev->udev,
                              usb_rcvbulkpipe (dev->udev,
                                               dev->ep8_address),
                              buffer,
                              max_packet_size,
                              &count,
                              HZ*10);

#ifdef PARSER_DEBUG
        printk(KERN_INFO "tmk1553busb: block_out_in: ofread %d\n", Result);
#endif

        if(Result)
        {
          err = 1;
          goto error;
        }

        load_buf_data_new(max_packet_size, buffer, buf_in, count_buf_in, &pointer);
      }
    } //if Ovf
  } //For

  if(out_enable && (incorrect_command == FALSE))
  {
    buffer[count_buffer_out] = 0xFF;
    buff_ovf = FALSE;
    count_buffer_in = 0;
    count_buffer_out = 0;
    out_enable = FALSE;

    Result = usb_bulk_msg(dev->udev,
                          usb_sndbulkpipe (dev->udev,
                                           dev->ep2_address),
                          buffer,
                          max_packet_size,
                          &count,
                          HZ*10);

#ifdef PARSER_DEBUG
    printk(KERN_INFO "tmk1553busb: block_out_in: normwrite %d\n", Result);
#endif

    if(Result)
    {
      err = 1;
      goto error;
    }

    if(in_enable)
    {
      in_enable = FALSE;

      Result = usb_bulk_msg(dev->udev,
                            usb_rcvbulkpipe (dev->udev,
                                             dev->ep8_address),
                            buffer,
                            max_packet_size,
                            &count,
                            HZ*10);

#ifdef PARSER_DEBUG
      printk(KERN_INFO "tmk1553busb: block_out_in: normread %d\n", Result);
#endif

      if(Result)
      {
        err = 1;
        goto error;
      }
      load_buf_data_new(max_packet_size, buffer, buf_in, count_buf_in, &pointer);
    }
#ifdef ASYNCHRONOUS_IO
    else if(cAsync[dev->minor])
    {
      Result = usb_control_msg(dev->udev,
                               usb_rcvctrlpipe(dev->udev, 0),
                               0xb9,
                               USB_DIR_IN,
                               0,
                               0,
                               dev->fwver,
                               2,
                               10*HZ);
      if(Result)
      {
        err = 1;
        goto error;
      }
    }
#endif
  }//if out en
error:
  kfree(buffer);
  return err;
}

void load_buf_data_new(u16 max_packet_size, u8 * buf, u16 * buf_data, u16 count_buf_in, u16 * pointer)
{
  unsigned int usb_pointer;  //buf - from USB, buf_data - to user
  int Index, count;

  for(usb_pointer = 0; ((*pointer) < count_buf_in) && (usb_pointer < max_packet_size); )
  {
    switch (buf_data[(*pointer)])
    {
      case 1:
        buf_data[(*pointer) + 2] = (buf[usb_pointer] << 8) | buf[usb_pointer + 1];
        (*pointer) += 3;
        usb_pointer += 2;
      break;
      case 3:
        count = buf_data[(*pointer) + 1];
        (*pointer) += 3;

        for(Index = 0; Index < count; Index++, usb_pointer += 2, (*pointer)++)
          buf_data[(*pointer)] = (buf[usb_pointer] << 8) | buf[usb_pointer + 1];

      break;
      case 11:
        if( (buf_data[(*pointer) + 1] > 0) && (buf_data[(*pointer) + 1] < 10) )
        {
          buf_data[(*pointer) + 2] = (buf[usb_pointer] << 8) | buf[usb_pointer + 1];
          (*pointer) += 3;
          usb_pointer += 2;
        }
        else
        {
          count = buf_data[(*pointer) + 2];
          (*pointer) += 4;

          for(Index = 0; Index < count; Index++, usb_pointer += 2, (*pointer)++)
            buf_data[(*pointer)] = (buf[usb_pointer] << 8) | buf[usb_pointer + 1];
        }
      break;
      default:
        (*pointer) = count_buf_in;
    }
  }
}

import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import { IconButton, Menu, MenuItem } from '@mui/material';

import Security from '@mui/icons-material/Security';

const useStyles = makeStyles({
  btnArea: {
    display: 'flex',
    marginTop: 10,
  },

  bannerBtn: {
    display: 'flex',
    height: 50,
    width: 50,

  },
});

export default function SecurityBtn() {
  const classes = useStyles();
  const navigate = useNavigate();
  const [anchor, setAnchor] = useState(null);

  const handleClickSecurity = (e) => {
    setAnchor(e.currentTarget);
  };

  const handleClose = () => {
    setAnchor(null);
  };

  const handleClickCP = () => {
    navigate('/security/password');
  };

  const handleClickDA = () => {
    navigate('/security/deactivate');
  };

  return (
    <div>
      <div className={classes.btnArea}>
        <IconButton className={classes.bannerBtn} onClick={handleClickSecurity}>
          <Security />
        </IconButton>
      </div>
      <Menu
        id="userMenu"
        anchorEl={anchor}
        open={Boolean(anchor)}
        onClose={handleClose}
        anchorOrigin={{
          vertical: 'bottom',
          horizontal: 'right',
        }}
        transformOrigin={{
          vertical: 'top',
          horizontal: 'right',
        }}
      >
        <MenuItem onClick={handleClickCP}>Change Password</MenuItem>
        <MenuItem onClick={handleClickDA}>Deactivate My Account</MenuItem>
      </Menu>
    </div>
  );
}

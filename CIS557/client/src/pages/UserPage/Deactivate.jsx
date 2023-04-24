import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Button, TextField, Paper,
} from '@mui/material';

import Layout from '../../components/Layout/Layout';

import * as security from '../../utils/security';

const useStyles = makeStyles({
  root: {
    background: '#FFF5E1',
  },

  paper: {
    display: 'inlined',
    width: 540,
    marginLeft: 'auto',
    marginRight: 'auto',
    justifyContent: 'center',
    textAlign: 'center',
    paddingBottom: 50,
    paddingTop: 50,
  },

  padding: {
    height: 50,
  },

  form: {
    width: 500,
  },

  btnArea: {
    marginLeft: 40,
  },

});

export default function Deactivate() {
  const classes = useStyles();
  const navigate = useNavigate();

  const userID = sessionStorage.getItem('userID');

  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');

  const [usernameError, setUsernameError] = useState(false);
  const [passwordError, setPasswordError] = useState(false);

  const [helperText, setHelperText] = useState('YOUR DATA WILL BE ERASED FROM DB!');

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    }
  }, []);

  const handleSubmit = async (e) => {
    e.preventDefault();
    setUsernameError(false);
    setPasswordError(false);
    if (username !== userID) {
      setUsernameError(true);
      setHelperText('Wrong Username.');
    } else {
      const res = await security.deactivate(userID, password);
      if (res === 400) {
        setPasswordError(true);
        setHelperText('Wrong password. Authentication failed.');
      } else if (res === 200) {
        sessionStorage.removeItem('userID');
        navigate('/');
      }
    }
  };

  return (
    <Layout>
      <div className={classes.root}>
        <div className={classes.padding} />
        <Paper className={classes.paper} elevation={3}>
          <Typography
            variant="h4"
            color="textPrimary"
          >
            Deactivate Your Account
          </Typography>
          <form className={classes.form} autoComplete="off" onSubmit={handleSubmit}>
            <TextField
              sx={{
                marginTop: '20px',
                marginLeft: '20px',
              }}
              variant="outlined"
              color="secondary"
              label="Username"
              error={usernameError}
              fullWidth
              inputProps={{ maxLength: 60 }}
              required
              onChange={(e) => setUsername(e.target.value)}
            />

            <TextField
              sx={{
                marginTop: '10px',
                marginBottom: '10px',
                marginLeft: '20px',
              }}
              variant="outlined"
              color="secondary"
              label="Password"
              type="password"
              error={passwordError}
              fullWidth
              inputProps={{ maxLength: 60 }}
              required
              onChange={(e) => setPassword(e.target.value)}
            />

            <Typography sx={{ marginLeft: '40px', marginBottom: '10px', color: '#9D2933' }} align="center">

              {helperText}

            </Typography>
            <div className={classes.btnArea}>
              <Button
                type="test"
                variant="outlined"
                color="secondary"
                align="center"
                sx={{
                  borderRadius: '30px',
                  border: '4px solid #9D2933',
                  fontSize: 25,
                  '&:hover': {
                    background: '#9D2933',
                    color: '#ffffff',
                    border: '4px solid #9D2933',
                  },
                }}
              >
                Confirm
              </Button>
            </div>
          </form>

        </Paper>
        <div className={classes.padding} />
      </div>
    </Layout>
  );
}

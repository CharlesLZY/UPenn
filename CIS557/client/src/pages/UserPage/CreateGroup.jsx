import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Container, TextField, Paper, Button, Typography, Switch,
  Radio, RadioGroup, FormControl, FormControlLabel,
} from '@mui/material';

import Layout from '../../components/Layout/Layout';

import * as fetchPublicGroup from '../../utils/fetchPublicGroup';
import * as fetchPrivateGroup from '../../utils/fetchPrivateGroup';
import * as utils from '../../utils/utils';

const useStyles = makeStyles({
  groupname: {
    marginTop: 40,
    display: 'flex',
    justifyContent: 'center',
  },

  private_label: {
    marginTop: 'auto',
    marginBottom: 'auto',
  },

  inputBar: {
    width: 400,
    marginTop: 'auto',
    marginBottom: 'auto',
  },

  paper: {
    width: 600,
    height: 450,
    display: 'inlined',
    // textAlign: 'center',
    marginLeft: 'auto',
    marginRight: 'auto',
  },

  tagArea: {
    display: 'flex',
  },

  tags: {
    marginTop: 40,
    marginLeft: 11,
  },
  banner: {
    paddingTop: 40,
  },

  padding: {
    height: '5vw',
  },

  privateSwitch: {
    display: 'flex',
    justifyContent: 'left',
    marginTop: 10,
    paddingLeft: 110,
  },

});

export default function CreateGroup() {
  const classes = useStyles();
  const navigate = useNavigate();

  const userID = sessionStorage.getItem('userID');

  const [input, setInput] = useState('');
  const [tag, setTag] = useState('life');

  const [privateGroup, setPrivate] = useState(false);
  const [nameError, setNameError] = useState(false);
  const [helperText, setHelperText] = useState('');

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    }
  }, []);

  const handleChange = (event) => {
    setTag(event.target.value);
  };

  const handleCreate = async () => {
    if (!utils.checkString(input)) {
      setNameError(true);
      setHelperText('Invalid group name.');
    } else if (!privateGroup) {
      const res = await fetchPublicGroup.createPublicGroup(input, userID, tag);
      if (res === 409) {
        setNameError(true);
        setHelperText('Group name existed.');
      } else {
        navigate('/userpage');
      }
    } else {
      const res = await fetchPrivateGroup.createPrivateGroup(input, userID, tag);
      if (res === 409) {
        setNameError(true);
        setHelperText('Group name existed.');
      } else {
        navigate('/userpage');
      }
    }
  };

  return (
    <Layout>
      <Container>
        <div className={classes.padding} />
        <Paper className={classes.paper} elevation={3}>
          <div className={classes.banner}>
            <Typography
              variant="h3"
              color="textPrimary"
              sx={{ marginLeft: '97px' }}
            >
              Create Your Group
            </Typography>
          </div>
          <div className={classes.groupname}>
            <TextField
              className={classes.inputBar}
              inputProps={{ maxLength: 45 }}
              size="small"
              variant="outlined"
              label="Group Name"
              color="secondary"
              error={nameError}
              helperText={helperText}
              onChange={(e) => {
                setInput(e.target.value);
                setHelperText('');
                setNameError(false);
              }}
            />
          </div>
          <div className={classes.tagArea}>
            <Typography sx={{ marginTop: '48px', marginLeft: '109px' }}>Tags :</Typography>
            <FormControl component="fieldset" sx={{ marginTop: '40px', marginLeft: '20px' }}>
              <RadioGroup
                row
                aria-label="gender"
                name="controlled-radio-buttons-group"
                value={tag}
                onChange={handleChange}
              >
                <FormControlLabel value="life" control={<Radio color="secondary" />} label="life" />
                <FormControlLabel value="study" control={<Radio color="secondary" />} label="study" />
                <FormControlLabel value="sport" control={<Radio color="secondary" />} label="sport" />
                <FormControlLabel value="game" control={<Radio color="secondary" />} label="game" />
              </RadioGroup>
            </FormControl>

          </div>
          <div className={classes.privateSwitch}>
            <Typography sx={{ display: 'flex', marginTop: '10px' }}>Private</Typography>
            <Switch sx={{ marginTop: '4px' }} checked={privateGroup} onChange={() => setPrivate(!privateGroup)} color="secondary" />
          </div>
          <br />
          <Button
            sx={{
              borderRadius: '30px',
              border: '4px solid #9D2933',
              fontSize: 25,
              '&:hover': {
                background: '#9D2933',
                color: '#ffffff',
                border: '4px solid #9D2933',
              },
              marginLeft: '235px',
            }}
            variant="outlined"
            color="secondary"
            align="center"
            onClick={handleCreate}
          >
            Create
          </Button>
        </Paper>
        <div className={classes.padding} />
      </Container>
    </Layout>
  );
}

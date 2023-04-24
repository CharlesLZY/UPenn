import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Paper, Button, Tab, Tabs,
} from '@mui/material';

import GroupList from './GroupList';

const useStyles = makeStyles({
  groupArea: {
    marginTop: '10px',
    marginLeft: '20px',
    width: 300,
    height: 480,
    textAlign: 'center',
    display: 'inlined',
  },

  btn: {
    position: 'absolute',
    marginLeft: 108,
    marginTop: 420,

  },
});

export default function GroupBar({ userInfo }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const [value, setValue] = useState(0);

  const handleClick = (e, val) => {
    setValue(val);
  };

  function TabPanel(props) {
    const { children, value, index } = props;
    return (
      <div>
        {value === index && <div>{children}</div>}
      </div>
    );
  }

  const handleClickCreate = () => {
    navigate('/creategroup');
  };

  return (
    <Paper className={classes.groupArea}>
      <div className={classes.btn}>
        <Button
          sx={{
            borderRadius: '30px',
            border: '2px solid #9D2933',
            fontSize: 15,
            '&:hover': {
              background: '#9D2933',
              color: '#ffffff',
              border: '2px solid #9D2933',
            },
          }}
          variant="outlined"
          color="secondary"
          align="center"
          onClick={handleClickCreate}
        >
          Create
        </Button>
      </div>
      <div>
        <Tabs value={value} variant="fullWidth" TabIndicatorProps={{ style: { background: '#ffffff' } }} onChange={handleClick}>
          <Tab label="Public" />
          <Tab label="Private" />
        </Tabs>
        <TabPanel value={value} index={0}>
          <GroupList groups={userInfo ? userInfo.publicgroups : []} groupType="publicgroup" />
        </TabPanel>
        <TabPanel value={value} index={1}>
          <GroupList groups={userInfo ? userInfo.privategroups : []} groupType="privategroup" />
        </TabPanel>

      </div>

    </Paper>
  );
}
